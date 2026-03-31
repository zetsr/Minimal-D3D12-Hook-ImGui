#include "mdx12_api.h"

namespace g_MDX12 {
    // Fonts
    // ImFont* g_Alibaba_PuHuiTi_Regular = nullptr;
    // ImFont* g_Alibaba_PuHuiTi_Bold = nullptr;
    // ImFont* g_Alibaba_PuHuiTi_Heavy = nullptr;
    // ImFont* g_Alibaba_PuHuiTi_Light = nullptr;
    ImFont* g_Alibaba_PuHuiTi_Medium = nullptr;

    // Hook original function pointers
    namespace g_HookFunctions {
        PFN_Present g_oPresent = nullptr;
        PFN_ExecuteCommandLists g_oExecuteCommandLists = nullptr;
        PFN_ResizeBuffers g_oResizeBuffers = nullptr;
        PFN_GetRawInputData g_oGetRawInputData = nullptr;
        PFN_GetRawInputBuffer g_oGetRawInputBuffer = nullptr;
        PFN_GetCursorPos g_oGetCursorPos = nullptr;
        PFN_SetCursorPos g_oSetCursorPos = nullptr;
        PFN_SetCursor g_oSetCursor = nullptr;
        PFN_ShowCursor g_oShowCursor = nullptr;
        PFN_GetClipCursor g_oGetClipCursor = nullptr;
        PFN_ClipCursor g_oClipCursor = nullptr;
        PFN_GetMouseMovePointsEx g_oGetMouseMovePointsEx = nullptr;
    }

    // 运行时模块加载
    namespace g_RuntimeModules {
        HMODULE g_hD3D12 = nullptr;
        HMODULE g_hDXGI = nullptr;
        PFN_D3D12CreateDevice g_pD3D12CreateDevice = nullptr;
        PFN_CreateDXGIFactory1 g_pCreateDXGIFactory1 = nullptr;

        // 使用 GetModuleHandleA 轮询，等待目标进程自然加载模块
        // 绝对不用 LoadLibraryA，避免在模块未就绪时强制加载导致崩溃
        bool WaitAndLoad() {
            while (true) {
                if (!g_hD3D12) {
                    g_hD3D12 = GetModuleHandleA("d3d12.dll");
                }
                if (!g_hDXGI) {
                    g_hDXGI = GetModuleHandleA("dxgi.dll");
                }

                if (g_hD3D12 && g_hDXGI) {
                    g_pD3D12CreateDevice = reinterpret_cast<PFN_D3D12CreateDevice>(
                        GetProcAddress(g_hD3D12, "D3D12CreateDevice"));
                    g_pCreateDXGIFactory1 = reinterpret_cast<PFN_CreateDXGIFactory1>(
                        GetProcAddress(g_hDXGI, "CreateDXGIFactory1"));

                    if (g_pD3D12CreateDevice && g_pCreateDXGIFactory1) {
                        return true;
                    }
                }

                Sleep(10);
            }
        }
    }

    // Direct3D 12 resources
    namespace g_D3D12Resources {
        ID3D12Device* g_pd3dDevice = nullptr;
        ID3D12CommandQueue* g_pd3dCommandQueue = nullptr;
        ID3D12DescriptorHeap* g_pd3dRtvDescHeap = nullptr;
        ID3D12DescriptorHeap* g_pd3dSrvDescHeap = nullptr;
        ID3D12GraphicsCommandList* g_pd3dCommandList = nullptr;
        ID3D12Fence* g_fence = nullptr;
        HANDLE g_fenceEvent = nullptr;
        UINT64 g_fenceValue = 0;
        UINT g_bufferCount = 0;
        std::vector<FrameContext> g_FrameContexts;
    }

    // Initialization state
    namespace g_InitState {
        bool g_Initialized = false;
        bool g_AfterFirstPresent = false;
        std::mutex g_InitMutex;
        UINT g_waitTimeoutMs = 2000;
    }

    // Process and window
    namespace g_ProcessWindow {
        std::string g_processName;
        HWND g_mainWindow = nullptr;
        RECT g_windowRect = { 0 };
        RECT g_cachedRect; 
        bool g_isFocused;
    }

    // Input state
    namespace g_InputState {
        std::atomic<bool> g_blockMouseInput{ false };
        std::atomic<bool> g_blockKeyboardInput{ false };
    }

    // Menu state
    namespace g_MenuState {
        bool g_isOpen = true;
        bool g_wasOpenLastFrame = true;
        POINT g_lastMousePos = { 0, 0 };
    }

    // Callback functions
    namespace g_Callbacks {
        SetupImGuiCallback g_setupImGuiCallback = nullptr;
    }
}