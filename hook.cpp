#include "hook.h"
#include "input.h"
#include "render.h"
#include "setup_imgui.h"

#pragma warning(push)
#pragma warning(disable: 26451)
#pragma warning(disable: 26812)
#pragma warning(disable: 6387)
#pragma warning(pop)

void STDMETHODCALLTYPE hkExecuteCommandLists(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists) {
    if (!g_pd3dCommandQueue && g_AfterFirstPresent && queue) {
        D3D12_COMMAND_QUEUE_DESC desc = queue->GetDesc();

        if (desc.Type == D3D12_COMMAND_LIST_TYPE_DIRECT) {
            ID3D12Device* tempDevice = nullptr;

            if (SUCCEEDED(queue->GetDevice(IID_PPV_ARGS(&tempDevice)))) {
                tempDevice->Release();
                queue->AddRef();
                g_pd3dCommandQueue = queue;
            }
        }
    }

    if (oExecuteCommandLists) oExecuteCommandLists(queue, NumCommandLists, ppCommandLists);
}

HRESULT STDMETHODCALLTYPE hkPresent(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags) {
    g_AfterFirstPresent = true;

    if (!g_pd3dCommandQueue) {
        if (oPresent) return oPresent(pSwapChain, SyncInterval, Flags);

        return S_OK;
    }

    std::lock_guard<std::mutex> lock(g_InitMutex);

    if (!g_Initialized) {
        ID3D12Device* deviceFromSwap = nullptr;

        if (FAILED(pSwapChain->GetDevice(IID_PPV_ARGS(&deviceFromSwap)))) {
            if (oPresent) return oPresent(pSwapChain, SyncInterval, Flags);

            return S_OK;
        }

        if (!g_pd3dDevice) g_pd3dDevice = deviceFromSwap;
        else deviceFromSwap->Release();

        DXGI_SWAP_CHAIN_DESC desc{};
        pSwapChain->GetDesc(&desc);
        g_bufferCount = desc.BufferCount;
        HWND newWindow = desc.OutputWindow;
        GetWindowRect(newWindow, &g_windowRect);

        if (g_mainWindow != newWindow) {
            if (g_mainWindow) {
                inputhook::Remove(g_mainWindow);
            }

            g_mainWindow = newWindow;
            inputhook::Init(g_mainWindow);
        }

        else if (!g_mainWindow) {
            g_mainWindow = newWindow;
            inputhook::Init(g_mainWindow);
        }

        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
        rtvHeapDesc.NumDescriptors = g_bufferCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        if (FAILED(g_pd3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&g_pd3dRtvDescHeap)))) {
            if (oPresent) return oPresent(pSwapChain, SyncInterval, Flags);
            return S_OK;
        }

        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{};
        srvHeapDesc.NumDescriptors = 1;
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

        if (FAILED(g_pd3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)))) {
            if (oPresent) return oPresent(pSwapChain, SyncInterval, Flags);
            return S_OK;
        }

        g_FrameContexts.resize(g_bufferCount);
        UINT rtvIncrementSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();

        for (UINT i = 0; i < g_bufferCount; ++i) {
            if (FAILED(g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_FrameContexts[i].CommandAllocator)))) {
                if (oPresent) return oPresent(pSwapChain, SyncInterval, Flags);

                return S_OK;
            }

            if (FAILED(pSwapChain->GetBuffer(i, IID_PPV_ARGS(&g_FrameContexts[i].Resource)))) {
                if (oPresent) return oPresent(pSwapChain, SyncInterval, Flags);

                return S_OK;
            }

            g_FrameContexts[i].Descriptor = rtvHandle;
            g_pd3dDevice->CreateRenderTargetView(g_FrameContexts[i].Resource, nullptr, rtvHandle);
            rtvHandle.ptr += static_cast<SIZE_T>(rtvIncrementSize);
        }

        if (FAILED(g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_FrameContexts[0].CommandAllocator, nullptr, IID_PPV_ARGS(&g_pd3dCommandList)))) {
            if (oPresent) return oPresent(pSwapChain, SyncInterval, Flags);

            return S_OK;
        }

        g_pd3dCommandList->Close();

        if (FAILED(g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)))) {
            if (oPresent) return oPresent(pSwapChain, SyncInterval, Flags);

            return S_OK;
        }

        g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

        if (!g_fenceEvent) {
            if (oPresent) return oPresent(pSwapChain, SyncInterval, Flags);

            return S_OK;
        }

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

        io.IniFilename = nullptr;
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

        ImGui::StyleColorsDark();
        ImGui_ImplWin32_Init(g_mainWindow);
        ImGui_ImplDX12_Init(g_pd3dDevice, g_bufferCount, desc.BufferDesc.Format, g_pd3dSrvDescHeap, g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(), g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

        unsigned char* pixels;
        int width, height;

        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        rawinputhook::Init();
        cursorhook::Init();

        g_Initialized = true;
    }

    UINT backBufferIdx = pSwapChain->GetCurrentBackBufferIndex();
    FrameContext& frameCtx = g_FrameContexts[backBufferIdx];

    if (g_fence && frameCtx.FenceValue != 0) {
        UINT64 completed = g_fence->GetCompletedValue();

        if (completed < frameCtx.FenceValue) {
            g_fence->SetEventOnCompletion(frameCtx.FenceValue, g_fenceEvent);
            WaitForSingleObject(g_fenceEvent, g_waitTimeoutMs);
        }
    }

    frameCtx.CommandAllocator->Reset();
    g_pd3dCommandList->Reset(frameCtx.CommandAllocator, nullptr);

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = frameCtx.Resource;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    g_pd3dCommandList->ResourceBarrier(1, &barrier);
    g_pd3dCommandList->OMSetRenderTargets(1, &frameCtx.Descriptor, FALSE, nullptr);

    ID3D12DescriptorHeap* ppHeaps[] = { g_pd3dSrvDescHeap };
    g_pd3dCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    bool cursorStateChanged = (menu::isOpen != menu::wasOpenLastFrame);

    menu::wasOpenLastFrame = menu::isOpen;
    inputhook::UpdateInputBlockState();
    cursorhook::UpdateCursorState();

    if (menu::isOpen) {
        io.MouseDrawCursor = true;
        io.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
    }
    else {
        io.MouseDrawCursor = false;
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    }

    if (cursorStateChanged) {
        if (menu::isOpen) {
            while (ShowCursor(FALSE) >= 0);
            SetCursor(nullptr);
            GetCursorPos(&menu::s_lastMousePos);
            RECT rect;
            GetWindowRect(g_mainWindow, &rect);
            int centerX = rect.left + (rect.right - rect.left) / 2;
            int centerY = rect.top + (rect.bottom - rect.top) / 2;
            SetCursorPos(centerX, centerY);
        }
    }

    if (menu::isOpen) {
        static int frameCounter = 0;
        frameCounter++;

        if (frameCounter % 30 == 0) {
            RECT rect;
            GetWindowRect(g_mainWindow, &rect);
            int centerX = rect.left + (rect.right - rect.left) / 2;
            int centerY = rect.top + (rect.bottom - rect.top) / 2;
            SetCursorPos(centerX, centerY);
        }
    }

    SetupImGui(pSwapChain, SyncInterval, Flags);
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    g_pd3dCommandList->ResourceBarrier(1, &barrier);
    g_pd3dCommandList->Close();

    if (g_pd3dCommandQueue) {
        if (oExecuteCommandLists) {
            oExecuteCommandLists(g_pd3dCommandQueue, 1, (ID3D12CommandList* const*)&g_pd3dCommandList);
        }

        UINT64 frameFence = ++g_fenceValue;
        frameCtx.FenceValue = frameFence;
        g_pd3dCommandQueue->Signal(g_fence, frameFence);
    }

    if (oPresent) return oPresent(pSwapChain, SyncInterval, Flags);

    return S_OK;
}

HRESULT STDMETHODCALLTYPE hkResizeBuffers(
    IDXGISwapChain* pSwapChain,
    UINT BufferCount,
    UINT Width,
    UINT Height,
    DXGI_FORMAT NewFormat,
    UINT SwapChainFlags)
{
    std::lock_guard<std::mutex> lock(g_InitMutex);

    if (g_Initialized) {
        CleanupRenderResources_NoInput();
    }

    HRESULT hr = oResizeBuffers ? oResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags) : S_OK;

    g_blockMouseInput = false;
    g_blockKeyboardInput = false;

    return hr;
}

DWORD WINAPI MainThread(LPVOID) {
    if (MH_Initialize() != MH_OK) return 0;
    while (true) {
        WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DefWindowProcW, 0, 0, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"TempDX12", nullptr };
        RegisterClassEx(&wc);
        HWND tempWnd = CreateWindow(wc.lpszClassName, L"Temp", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, nullptr, nullptr, wc.hInstance, nullptr);
        if (!tempWnd) { UnregisterClass(wc.lpszClassName, wc.hInstance); Sleep(1); continue; }
        ID3D12Device* tempDevice = nullptr;
        ID3D12CommandQueue* tempQueue = nullptr;
        IDXGIFactory4* factory = nullptr;
        IDXGISwapChain* tempSwapChain = nullptr;

        bool ok = SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&tempDevice)));

        if (ok) {
            D3D12_COMMAND_QUEUE_DESC qdesc = {};
            qdesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            qdesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
            qdesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            qdesc.NodeMask = 0;
            ok = SUCCEEDED(tempDevice->CreateCommandQueue(&qdesc, IID_PPV_ARGS(&tempQueue)));
        }

        if (ok) {
            ok = SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));
        }

        if (ok) {
            DXGI_SWAP_CHAIN_DESC sd = {};
            sd.BufferCount = 2;
            sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.OutputWindow = tempWnd;
            sd.SampleDesc.Count = 1;
            sd.Windowed = TRUE;
            sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            ok = SUCCEEDED(factory->CreateSwapChain(tempQueue, &sd, &tempSwapChain));
        }

        if (ok) {
            void** swapVTable = *reinterpret_cast<void***>(tempSwapChain);
            void** queueVTable = *reinterpret_cast<void***>(tempQueue);

            if (swapVTable && swapVTable[8]) {
                MH_CreateHook(swapVTable[8], reinterpret_cast<LPVOID>(hkPresent), reinterpret_cast<LPVOID*>(&oPresent));
            }

            if (swapVTable && swapVTable[13]) {
                MH_CreateHook(swapVTable[13], reinterpret_cast<LPVOID>(hkResizeBuffers), reinterpret_cast<LPVOID*>(&oResizeBuffers));
            }

            if (queueVTable && queueVTable[10]) {
                MH_CreateHook(queueVTable[10], reinterpret_cast<LPVOID>(hkExecuteCommandLists), reinterpret_cast<LPVOID*>(&oExecuteCommandLists));
            }

            MH_EnableHook(MH_ALL_HOOKS);
            tempSwapChain->Release();
            factory->Release();
            tempQueue->Release();
            tempDevice->Release();
            DestroyWindow(tempWnd);
            UnregisterClass(wc.lpszClassName, wc.hInstance);

            break;
        }

        if (tempSwapChain) tempSwapChain->Release();
        if (factory) factory->Release();
        if (tempQueue) tempQueue->Release();
        if (tempDevice) tempDevice->Release();

        DestroyWindow(tempWnd);
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        Sleep(1);
    }

    return 0;
}
