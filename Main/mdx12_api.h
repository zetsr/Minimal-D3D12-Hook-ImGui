#pragma once

#include <windows.h>
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <vector>
#include <mutex>
#include <string>
#include <atomic>
#include <unordered_set>

#include "../MinHook/include/MinHook.h"

// #include "../Font/Alibaba-PuHuiTi-Bold.h"
// #include "../Font/Alibaba-PuHuiTi-Heavy.h"
// #include "../Font/Alibaba-PuHuiTi-Light.h"
#include "../Font/Alibaba-PuHuiTi-Medium.h"
// #include "../Font/Alibaba-PuHuiTi-Regular.h"

#include "../ImGui/imgui.h"
#include "../ImGui/imgui_internal.h"
#include "../ImGui/imgui_impl_win32.h"
#include "../ImGui/imgui_impl_dx12.h"

// 此处承载第三方库（ImGui后端等）所需的静态链接
// 我们自己的 hook 代码通过运行时 GetProcAddress 调用 D3D12/DXGI，不依赖这些符号
// 但 imgui_impl_dx12 内部引用了 CreateDXGIFactory1 等符号，必须保留链接
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "user32.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// 运行时动态加载的D3D12/DXGI函数指针类型
typedef HRESULT(WINAPI* PFN_D3D12CreateDevice)(IUnknown*, D3D_FEATURE_LEVEL, REFIID, void**);
typedef HRESULT(WINAPI* PFN_CreateDXGIFactory1)(REFIID, void**);

// Main namespace for all globals
namespace g_MDX12 {
    // Fonts
    // inline ImFont* g_Alibaba_PuHuiTi_Regular = nullptr;
    inline ImFont* g_Alibaba_PuHuiTi_Bold = nullptr;
    // inline ImFont* g_Alibaba_PuHuiTi_Heavy = nullptr;
    // inline ImFont* g_Alibaba_PuHuiTi_Light = nullptr;
    inline ImFont* g_Alibaba_PuHuiTi_Medium = nullptr;
    inline ImFont* g_icomoon = nullptr;
    inline ImFont* g_icomoon_small = nullptr;
    inline ImFont* g_icomoon_big = nullptr;

    // Hook function pointer types
    typedef HRESULT(STDMETHODCALLTYPE* PFN_Present)(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags);
    typedef void(STDMETHODCALLTYPE* PFN_ExecuteCommandLists)(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);
    typedef HRESULT(STDMETHODCALLTYPE* PFN_ResizeBuffers)(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
    typedef UINT(WINAPI* PFN_GetRawInputData)(HRAWINPUT hRawInput, UINT uiCommand, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader);
    typedef UINT(WINAPI* PFN_GetRawInputBuffer)(PRAWINPUT pData, PUINT pcbSize, UINT cbSizeHeader);
    typedef BOOL(WINAPI* PFN_GetCursorPos)(LPPOINT lpPoint);
    typedef BOOL(WINAPI* PFN_SetCursorPos)(int X, int Y);
    typedef HCURSOR(WINAPI* PFN_SetCursor)(HCURSOR hCursor);
    typedef int(WINAPI* PFN_ShowCursor)(BOOL bShow);
    typedef BOOL(WINAPI* PFN_GetClipCursor)(LPRECT lpRect);
    typedef BOOL(WINAPI* PFN_ClipCursor)(const RECT* lpRect);
    typedef BOOL(WINAPI* PFN_GetMouseMovePointsEx)(UINT cbSize, LPMOUSEMOVEPOINT lppt, LPMOUSEMOVEPOINT lpptBuf, int nBufPoints, DWORD resolution);

    // Hook original function pointers namespace
    namespace g_HookFunctions {
        inline PFN_Present g_oPresent = nullptr;
        inline PFN_ExecuteCommandLists g_oExecuteCommandLists = nullptr;
        inline PFN_ResizeBuffers g_oResizeBuffers = nullptr;
        inline PFN_GetRawInputData g_oGetRawInputData = nullptr;
        inline PFN_GetRawInputBuffer g_oGetRawInputBuffer = nullptr;
        inline PFN_GetCursorPos g_oGetCursorPos = nullptr;
        inline PFN_SetCursorPos g_oSetCursorPos = nullptr;
        inline PFN_SetCursor g_oSetCursor = nullptr;
        inline PFN_ShowCursor g_oShowCursor = nullptr;
        inline PFN_GetClipCursor g_oGetClipCursor = nullptr;
        inline PFN_ClipCursor g_oClipCursor = nullptr;
        inline PFN_GetMouseMovePointsEx g_oGetMouseMovePointsEx = nullptr;
    }

    // 运行时动态加载的模块与函数指针
    // 用于在 MainThread 中安全地等待目标进程加载 d3d12.dll/dxgi.dll 后再操作
    namespace g_RuntimeModules {
        inline HMODULE g_hD3D12 = nullptr;
        inline HMODULE g_hDXGI = nullptr;
        inline PFN_D3D12CreateDevice g_pD3D12CreateDevice = nullptr;
        inline PFN_CreateDXGIFactory1 g_pCreateDXGIFactory1 = nullptr;

        // 使用 GetModuleHandleA 轮询，等待目标进程自然加载模块
        // 绝对不用 LoadLibraryA，避免在模块未就绪时强制加载导致崩溃
        inline bool WaitAndLoad() {
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

                Sleep(1);
            }
        }
    }

    // Direct3D 12 resources namespace
    namespace g_D3D12Resources {
        struct FrameContext {
            ID3D12CommandAllocator* CommandAllocator = nullptr;
            ID3D12Resource* Resource = nullptr;
            D3D12_CPU_DESCRIPTOR_HANDLE Descriptor{};
            UINT64 FenceValue = 0;
        };

        inline ID3D12Device* g_pd3dDevice = nullptr;
        inline ID3D12CommandQueue* g_pd3dCommandQueue = nullptr;
        inline ID3D12DescriptorHeap* g_pd3dRtvDescHeap = nullptr;
        inline ID3D12DescriptorHeap* g_pd3dSrvDescHeap = nullptr;
        inline ID3D12GraphicsCommandList* g_pd3dCommandList = nullptr;
        inline ID3D12Fence* g_fence = nullptr;
        inline HANDLE g_fenceEvent = nullptr;
        inline UINT64 g_fenceValue = 0;
        inline UINT g_bufferCount = 0;
        inline std::vector<FrameContext> g_FrameContexts;
    }

    // Initialization state namespace
    namespace g_InitState {
        inline bool g_Initialized = false;
        inline bool g_AfterFirstPresent = false;
        inline std::mutex g_InitMutex;
        inline UINT g_waitTimeoutMs = 30000;
    }

    // Process and window namespace
    namespace g_ProcessWindow {
        inline std::string g_processName;
        inline HWND g_mainWindow = nullptr;
        inline RECT g_windowRect = { 0 };
        inline RECT g_cachedRect;
        inline bool g_isFocused;
    }

    // Input state namespace
    namespace g_InputState {
        inline std::atomic<bool> g_blockMouseInput{ false };
        inline std::atomic<bool> g_blockKeyboardInput{ false };
    }

    // Menu state namespace
    namespace g_MenuState {
        inline bool g_isOpen = true;
        inline UINT g_openKey = VK_F1;
        inline UINT* g_pCurrentBindingKey = nullptr;
        inline bool g_bindingFinished = false;
        inline bool g_wasOpenLastFrame = true;
        inline POINT g_lastMousePos = { 0, 0 };
    }

    // Callback function type for custom ImGui drawing
    typedef void(*SetupImGuiCallback)(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags);

    namespace g_Callbacks {
        inline SetupImGuiCallback g_setupImGuiCallback = nullptr;
    }

    // Declarations
    namespace cursorhook {
        inline void Init();
        inline void Remove();
        inline void UpdateCursorState();
    }

    namespace rawinputhook {
        inline void Init();
        inline void Remove();
    }

    namespace inputhook {
        inline void Init(HWND hWindow);
        inline void Remove(HWND hWindow);
        inline void UpdateInputBlockState();
        inline void ReinstallWindowHook();
    }

    inline void CleanupRenderResources_NoInput();
    inline void FinalCleanupAll();
    inline void SetupImGui(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags);
    inline void SetSetupImGuiCallback(SetupImGuiCallback callback);
    inline DWORD WINAPI MainThread(LPVOID lpParam);
    inline void Initialize(LPVOID lpParam);

    // Cursor Hook Implementation
    namespace cursorhook {
        inline int g_cursorShowCount = 0;
        inline HCURSOR g_lastCursor = nullptr;
        inline bool g_initialized = false;
        inline POINT g_lastReportedPos = { 0, 0 };

        inline BOOL WINAPI hkGetCursorPos(LPPOINT lpPoint) {
            if (!lpPoint) {
                if (g_HookFunctions::g_oGetCursorPos) return g_HookFunctions::g_oGetCursorPos(nullptr);
                return FALSE;
            }

            if (g_MenuState::g_isOpen && g_InputState::g_blockMouseInput) {
                RECT rect;
                GetWindowRect(g_ProcessWindow::g_mainWindow, &rect);
                lpPoint->x = rect.left + (rect.right - rect.left) / 2;
                lpPoint->y = rect.top + (rect.bottom - rect.top) / 2;
                g_lastReportedPos = *lpPoint;
                return TRUE;
            }

            BOOL result = g_HookFunctions::g_oGetCursorPos ? g_HookFunctions::g_oGetCursorPos(lpPoint) : FALSE;

            if (result) {
                g_lastReportedPos = *lpPoint;
            }

            return result;
        }

        inline BOOL WINAPI hkSetCursorPos(int X, int Y) {
            if (g_MenuState::g_isOpen && g_InputState::g_blockMouseInput) {
                return TRUE;
            }

            return g_HookFunctions::g_oSetCursorPos ? g_HookFunctions::g_oSetCursorPos(X, Y) : FALSE;
        }

        inline HCURSOR WINAPI hkSetCursor(HCURSOR hCursor) {
            if (hCursor) {
                g_lastCursor = hCursor;
            }

            if (g_MenuState::g_isOpen) {
                // 菜单开启期间，阻止游戏通过 SetCursor(NULL) 隐藏光标
                if (hCursor == nullptr) {
                    return nullptr;
                }
            }

            return g_HookFunctions::g_oSetCursor ? g_HookFunctions::g_oSetCursor(hCursor) : nullptr;
        }

        inline int WINAPI hkShowCursor(BOOL bShow) {
            g_cursorShowCount += bShow ? 1 : -1;

            if (g_MenuState::g_isOpen) {
                return g_cursorShowCount;
            }

            return g_HookFunctions::g_oShowCursor ? g_HookFunctions::g_oShowCursor(bShow) : 0;
        }

        inline BOOL WINAPI hkClipCursor(const RECT* lpRect) {
            if (g_MenuState::g_isOpen) {
                return g_HookFunctions::g_oClipCursor ? g_HookFunctions::g_oClipCursor(nullptr) : FALSE;
            }

            return g_HookFunctions::g_oClipCursor ? g_HookFunctions::g_oClipCursor(lpRect) : FALSE;
        }

        inline BOOL WINAPI hkGetMouseMovePointsEx(UINT cbSize, LPMOUSEMOVEPOINT lppt, LPMOUSEMOVEPOINT lpptBuf, int nBufPoints, DWORD resolution) {
            if (g_MenuState::g_isOpen && g_InputState::g_blockMouseInput) {
                if (lppt) {
                    memset(lppt, 0, cbSize);
                }
                return 0;
            }

            return g_HookFunctions::g_oGetMouseMovePointsEx ? g_HookFunctions::g_oGetMouseMovePointsEx(cbSize, lppt, lpptBuf, nBufPoints, resolution) : 0;
        }

        inline void UpdateCursorState() {
            if (g_MenuState::g_isOpen) {
                if (g_HookFunctions::g_oShowCursor) {
                    while (g_HookFunctions::g_oShowCursor(TRUE) < 0);
                }

                if (g_HookFunctions::g_oClipCursor) {
                    g_HookFunctions::g_oClipCursor(nullptr);
                }

                if (g_HookFunctions::g_oSetCursor) {
                    g_HookFunctions::g_oSetCursor(g_lastCursor ? g_lastCursor : LoadCursor(nullptr, IDC_ARROW));
                }
            }
            else {
                if (g_cursorShowCount >= 0) {
                    if (g_HookFunctions::g_oShowCursor) {
                        while (g_HookFunctions::g_oShowCursor(TRUE) < 0);
                    }

                    if (g_HookFunctions::g_oSetCursor) {
                        g_HookFunctions::g_oSetCursor(g_lastCursor ? g_lastCursor : LoadCursor(nullptr, IDC_ARROW));
                    }
                }
                else {
                    if (g_HookFunctions::g_oShowCursor) {
                        while (g_HookFunctions::g_oShowCursor(FALSE) >= 0);
                    }

                    if (g_HookFunctions::g_oSetCursor) {
                        g_HookFunctions::g_oSetCursor(nullptr);
                    }
                }
            }
        }

        inline void Init() {
            HMODULE user32 = GetModuleHandleA("user32.dll");
            if (!user32) return;

            FARPROC getCursorPosAddr = GetProcAddress(user32, "GetCursorPos");
            FARPROC setCursorPosAddr = GetProcAddress(user32, "SetCursorPos");
            FARPROC setCursorAddr = GetProcAddress(user32, "SetCursor");
            FARPROC showCursorAddr = GetProcAddress(user32, "ShowCursor");
            FARPROC clipCursorAddr = GetProcAddress(user32, "ClipCursor");
            FARPROC getMouseMovePointsExAddr = GetProcAddress(user32, "GetMouseMovePointsEx");
            FARPROC getClipCursorAddr = GetProcAddress(user32, "GetClipCursor");

            if (getCursorPosAddr) {
                MH_CreateHook(getCursorPosAddr, reinterpret_cast<LPVOID>(hkGetCursorPos), reinterpret_cast<LPVOID*>(&g_HookFunctions::g_oGetCursorPos));
            }

            if (setCursorPosAddr) {
                MH_CreateHook(setCursorPosAddr, reinterpret_cast<LPVOID>(hkSetCursorPos), reinterpret_cast<LPVOID*>(&g_HookFunctions::g_oSetCursorPos));
            }

            if (setCursorAddr) {
                MH_CreateHook(setCursorAddr, reinterpret_cast<LPVOID>(hkSetCursor), reinterpret_cast<LPVOID*>(&g_HookFunctions::g_oSetCursor));
            }

            if (showCursorAddr) {
                MH_CreateHook(showCursorAddr, reinterpret_cast<LPVOID>(hkShowCursor), reinterpret_cast<LPVOID*>(&g_HookFunctions::g_oShowCursor));
            }

            if (clipCursorAddr) {
                MH_CreateHook(clipCursorAddr, reinterpret_cast<LPVOID>(hkClipCursor), reinterpret_cast<LPVOID*>(&g_HookFunctions::g_oClipCursor));
            }

            if (getMouseMovePointsExAddr) {
                MH_CreateHook(getMouseMovePointsExAddr, reinterpret_cast<LPVOID>(hkGetMouseMovePointsEx), reinterpret_cast<LPVOID*>(&g_HookFunctions::g_oGetMouseMovePointsEx));
            }

            if (getClipCursorAddr) {
                g_HookFunctions::g_oGetClipCursor = reinterpret_cast<PFN_GetClipCursor>(getClipCursorAddr);
            }

            if (g_HookFunctions::g_oGetCursorPos) {
                g_HookFunctions::g_oGetCursorPos(&g_lastReportedPos);
            }
            else {
                GetCursorPos(&g_lastReportedPos);
            }

            if (!g_lastCursor) {
                g_lastCursor = LoadCursor(nullptr, IDC_ARROW);
            }

            MH_EnableHook(MH_ALL_HOOKS);
            g_initialized = true;
        }

        inline void Remove() {
            if (!g_initialized) return;

            HMODULE user32 = GetModuleHandleA("user32.dll");
            if (!user32) return;

            FARPROC getCursorPosAddr = GetProcAddress(user32, "GetCursorPos");
            FARPROC setCursorPosAddr = GetProcAddress(user32, "SetCursorPos");
            FARPROC setCursorAddr = GetProcAddress(user32, "SetCursor");
            FARPROC showCursorAddr = GetProcAddress(user32, "ShowCursor");
            FARPROC clipCursorAddr = GetProcAddress(user32, "ClipCursor");
            FARPROC getMouseMovePointsExAddr = GetProcAddress(user32, "GetMouseMovePointsEx");

            if (getCursorPosAddr) MH_DisableHook(getCursorPosAddr);
            if (setCursorPosAddr) MH_DisableHook(setCursorPosAddr);
            if (setCursorAddr) MH_DisableHook(setCursorAddr);
            if (showCursorAddr) MH_DisableHook(showCursorAddr);
            if (clipCursorAddr) MH_DisableHook(clipCursorAddr);
            if (getMouseMovePointsExAddr) MH_DisableHook(getMouseMovePointsExAddr);

            g_HookFunctions::g_oGetCursorPos = nullptr;
            g_HookFunctions::g_oSetCursorPos = nullptr;
            g_HookFunctions::g_oSetCursor = nullptr;
            g_HookFunctions::g_oShowCursor = nullptr;
            g_HookFunctions::g_oClipCursor = nullptr;
            g_HookFunctions::g_oGetClipCursor = nullptr;
            g_HookFunctions::g_oGetMouseMovePointsEx = nullptr;
            g_initialized = false;
        }
    }

    // Raw Input Hook Implementation
    namespace rawinputhook {
        inline UINT WINAPI hkGetRawInputData(HRAWINPUT hRawInput, UINT uiCommand, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader) {
            if (g_MenuState::g_isOpen && g_InputState::g_blockMouseInput) {
                if (pcbSize) *pcbSize = 0;
                if (pData && pcbSize && *pcbSize > 0) {
                    memset(pData, 0, *pcbSize);
                }
                return 0;
            }

            return g_HookFunctions::g_oGetRawInputData ? g_HookFunctions::g_oGetRawInputData(hRawInput, uiCommand, pData, pcbSize, cbSizeHeader) : 0;
        }

        inline UINT WINAPI hkGetRawInputBuffer(PRAWINPUT pData, PUINT pcbSize, UINT cbSizeHeader) {
            if (g_MenuState::g_isOpen && g_InputState::g_blockMouseInput) {
                if (pcbSize) {
                    *pcbSize = 0;
                }

                if (pData && pcbSize && *pcbSize > 0) {
                    memset(pData, 0, *pcbSize);
                }

                return 0;
            }

            return g_HookFunctions::g_oGetRawInputBuffer ? g_HookFunctions::g_oGetRawInputBuffer(pData, pcbSize, cbSizeHeader) : 0;
        }

        inline void Init() {
            HMODULE user32 = GetModuleHandleA("user32.dll");
            if (!user32) return;

            FARPROC getRawInputDataAddr = GetProcAddress(user32, "GetRawInputData");
            FARPROC getRawInputBufferAddr = GetProcAddress(user32, "GetRawInputBuffer");

            if (getRawInputDataAddr) {
                MH_CreateHook(getRawInputDataAddr, reinterpret_cast<LPVOID>(hkGetRawInputData), reinterpret_cast<LPVOID*>(&g_HookFunctions::g_oGetRawInputData));
            }

            if (getRawInputBufferAddr) {
                MH_CreateHook(getRawInputBufferAddr, reinterpret_cast<LPVOID>(hkGetRawInputBuffer), reinterpret_cast<LPVOID*>(&g_HookFunctions::g_oGetRawInputBuffer));
            }

            if (getRawInputDataAddr) MH_EnableHook(getRawInputDataAddr);
            if (getRawInputBufferAddr) MH_EnableHook(getRawInputBufferAddr);
        }

        inline void Remove() {
            HMODULE user32 = GetModuleHandleA("user32.dll");
            if (!user32) return;

            FARPROC getRawInputDataAddr = GetProcAddress(user32, "GetRawInputData");
            FARPROC getRawInputBufferAddr = GetProcAddress(user32, "GetRawInputBuffer");

            if (getRawInputDataAddr && g_HookFunctions::g_oGetRawInputData) {
                MH_DisableHook(getRawInputDataAddr);
                MH_RemoveHook(getRawInputDataAddr);
            }

            if (getRawInputBufferAddr && g_HookFunctions::g_oGetRawInputBuffer) {
                MH_DisableHook(getRawInputBufferAddr);
                MH_RemoveHook(getRawInputBufferAddr);
            }

            g_HookFunctions::g_oGetRawInputData = nullptr;
            g_HookFunctions::g_oGetRawInputBuffer = nullptr;
        }
    }

    // Window/Input Hook Implementation
    namespace inputhook {
        inline WNDPROC sOriginalWndProc = nullptr;
        inline bool g_f1Down = false;

        inline void UpdateInputBlockState() {
            HWND foreground = GetForegroundWindow();
            bool isGameForeground = (foreground == g_ProcessWindow::g_mainWindow);
            char className[256] = { 0 };

            if (foreground && foreground != g_ProcessWindow::g_mainWindow) {
                GetClassNameA(foreground, className, sizeof(className));
            }

            bool gameHasMenuOpen = false;

            if (foreground && foreground != g_ProcessWindow::g_mainWindow) {
                DWORD pid;
                GetWindowThreadProcessId(foreground, &pid);
                DWORD gamePid = GetCurrentProcessId();

                if (pid == gamePid) {
                    gameHasMenuOpen = true;
                }
            }

            g_InputState::g_blockMouseInput = (g_MenuState::g_isOpen && !gameHasMenuOpen);
        }

        inline LRESULT APIENTRY WndProcHook(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
            // 如果当前正在录制任意按键（指针不为空）
            if (g_MenuState::g_pCurrentBindingKey != nullptr) {
                if (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) {
                    UINT vk = (UINT)wParam;

                    // 排除鼠标误触
                    if (vk != VK_LBUTTON && vk != VK_RBUTTON && vk != VK_MBUTTON) {
                        if (vk == VK_ESCAPE) {
                            // 按 ESC 取消录制
                            g_MenuState::g_pCurrentBindingKey = nullptr;
                        }
                        else {
                            // 直接向指针指向的内存写入捕获的原生虚拟键码
                            *g_MenuState::g_pCurrentBindingKey = vk;

                            g_MenuState::g_pCurrentBindingKey = nullptr;  // 录制结束，清空指针
                            g_MenuState::g_bindingFinished = true;        // 激活弹起保护
                        }
                        return 0; // 拦截，不响应游戏和 ImGui
                    }
                }
                // 录制期间拦截所有键盘杂音
                if (uMsg == WM_KEYUP || uMsg == WM_SYSKEYUP || uMsg == WM_CHAR) {
                    return 0;
                }
            }

            // 清除录制那一瞬间的按键弹起消息
            if (g_MenuState::g_bindingFinished) {
                if (uMsg == WM_KEYUP || uMsg == WM_SYSKEYUP) {
                    g_MenuState::g_bindingFinished = false;
                    return 0;
                }
            }

            if (uMsg == WM_KEYDOWN && wParam == g_MenuState::g_openKey && !g_f1Down) {
                g_f1Down = true;
                g_MenuState::g_isOpen = !g_MenuState::g_isOpen;

                // 状态重置
                if (ImGui::GetCurrentContext() != nullptr) {
                    ImGuiIO& io = ImGui::GetIO();

                    if (g_MenuState::g_isOpen) {
                        // 开启菜单时：清除鼠标按键状态，防止带入之前的点击或拖拽指令
                        io.ClearInputKeys();
                        // 强制将当前帧的鼠标位置设置为系统真实位置，防止 ImGui 沿用上一帧的缓存坐标
                        POINT p;
                        if (g_HookFunctions::g_oGetCursorPos) g_HookFunctions::g_oGetCursorPos(&p);
                        else GetCursorPos(&p);
                        ScreenToClient(hwnd, &p);
                        io.MousePos = ImVec2((float)p.x, (float)p.y);
                        io.MouseDrawCursor = false;
                        io.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
                    }
                    else {
                        // 关闭菜单时：将 ImGui 的鼠标坐标移出屏幕，防止关闭瞬间 ImGui 还在触发 Hover 状态导致系统光标视觉撕裂
                        io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
                        for (int i = 0; i < ImGuiMouseButton_COUNT; i++) {
                            io.MouseDown[i] = false;
                        }
                        io.MouseDrawCursor = false;
                        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
                    }
                }

                // 处理 ClipCursor 和 ShowCursor
                UpdateInputBlockState();
                cursorhook::UpdateCursorState();

                return 0;
            }
            else if (uMsg == WM_KEYUP && wParam == g_MenuState::g_openKey) {
                g_f1Down = false;
                return 0;
            }

            if (uMsg == WM_INPUT || uMsg == WM_INPUT_DEVICE_CHANGE) {
                if (g_MenuState::g_isOpen && g_InputState::g_blockMouseInput) {
                    return 0;
                }
            }

            if (g_MenuState::g_isOpen && ImGui::GetCurrentContext() != nullptr) {
                if (uMsg == WM_SETCURSOR) {
                    if (LOWORD(lParam) == HTCLIENT) {
                        if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam)) {
                            return TRUE;
                        }
                        SetCursor(cursorhook::g_lastCursor ? cursorhook::g_lastCursor : LoadCursor(nullptr, IDC_ARROW));
                        return TRUE;
                    }
                    return DefWindowProc(hwnd, uMsg, wParam, lParam);
                }

                ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);
                ImGuiIO& io = ImGui::GetIO();

                if (g_InputState::g_blockMouseInput) {
                    switch (uMsg) {
                    case WM_MOUSEMOVE:
                    case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_LBUTTONDBLCLK:
                    case WM_RBUTTONDOWN: case WM_RBUTTONUP: case WM_RBUTTONDBLCLK:
                    case WM_MBUTTONDOWN: case WM_MBUTTONUP: case WM_MBUTTONDBLCLK:
                    case WM_MOUSEWHEEL: case WM_MOUSEHWHEEL:
                    case WM_XBUTTONDOWN: case WM_XBUTTONUP: case WM_XBUTTONDBLCLK:
                        return 0;
                    }
                }

                if (io.WantCaptureKeyboard) {
                    switch (uMsg) {
                    case WM_KEYDOWN: case WM_KEYUP:
                    case WM_SYSKEYDOWN: case WM_SYSKEYUP:
                    case WM_CHAR: case WM_SYSCHAR:
                    case WM_DEADCHAR: case WM_SYSDEADCHAR:
                    case WM_HOTKEY:
                        return 0;
                    case WM_IME_SETCONTEXT:
                    case WM_IME_NOTIFY:
                    case WM_IME_STARTCOMPOSITION:
                    case WM_IME_ENDCOMPOSITION:
                    case WM_IME_COMPOSITION:
                    case WM_IME_CHAR:
                        return 0;
                    }
                }
            }

            if (sOriginalWndProc) {
                return CallWindowProc(sOriginalWndProc, hwnd, uMsg, wParam, lParam);
            }
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }

        inline void Init(HWND hWindow) {
            if (!hWindow) return;
            g_ProcessWindow::g_mainWindow = hWindow;

            if (sOriginalWndProc) {
                Remove(hWindow);
            }

            sOriginalWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProcHook)));
        }

        inline void Remove(HWND hWindow) {
            if (!hWindow || !sOriginalWndProc) return;

            SetWindowLongPtr(hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(sOriginalWndProc));
            sOriginalWndProc = nullptr;
        }

        inline void ReinstallWindowHook() {
            if (!g_ProcessWindow::g_mainWindow) return;

            if (!sOriginalWndProc) {
                Init(g_ProcessWindow::g_mainWindow);
            }
        }
    }

    inline void CleanupRenderResources_NoInput() {
        if (g_D3D12Resources::g_pd3dCommandQueue && g_D3D12Resources::g_fence && g_D3D12Resources::g_fenceEvent) {
            UINT64 localFence = ++g_D3D12Resources::g_fenceValue;

            if (SUCCEEDED(g_D3D12Resources::g_pd3dCommandQueue->Signal(g_D3D12Resources::g_fence, localFence))) {
                if (g_D3D12Resources::g_fence->GetCompletedValue() < localFence) {

                    g_D3D12Resources::g_fence->SetEventOnCompletion(localFence, g_D3D12Resources::g_fenceEvent);
                    WaitForSingleObject(g_D3D12Resources::g_fenceEvent, g_InitState::g_waitTimeoutMs);
                }
            }
        }

        if (ImGui::GetCurrentContext()) {
            ImGui_ImplDX12_InvalidateDeviceObjects();
        }

        if (g_D3D12Resources::g_pd3dCommandList) { g_D3D12Resources::g_pd3dCommandList->Release(); g_D3D12Resources::g_pd3dCommandList = nullptr; }
        if (g_D3D12Resources::g_pd3dRtvDescHeap) { g_D3D12Resources::g_pd3dRtvDescHeap->Release(); g_D3D12Resources::g_pd3dRtvDescHeap = nullptr; }
        if (g_D3D12Resources::g_pd3dSrvDescHeap) { g_D3D12Resources::g_pd3dSrvDescHeap->Release(); g_D3D12Resources::g_pd3dSrvDescHeap = nullptr; }
        if (g_D3D12Resources::g_fence) { g_D3D12Resources::g_fence->Release(); g_D3D12Resources::g_fence = nullptr; }
        if (g_D3D12Resources::g_fenceEvent) { CloseHandle(g_D3D12Resources::g_fenceEvent); g_D3D12Resources::g_fenceEvent = nullptr; }

        for (auto& frame : g_D3D12Resources::g_FrameContexts) {
            if (frame.Resource) { frame.Resource->Release(); frame.Resource = nullptr; }
            if (frame.CommandAllocator) { frame.CommandAllocator->Release(); frame.CommandAllocator = nullptr; }

            frame.FenceValue = 0;
        }

        g_D3D12Resources::g_FrameContexts.clear();
        g_D3D12Resources::g_bufferCount = 0;
        g_InitState::g_Initialized = false;
    }

    // Setup ImGui Callback Implementation
    inline void SetSetupImGuiCallback(SetupImGuiCallback callback) {
        g_Callbacks::g_setupImGuiCallback = callback;
    }

    inline void SetupImGui(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags) {
        // Call user-defined callback if set
        if (g_Callbacks::g_setupImGuiCallback) {
            g_Callbacks::g_setupImGuiCallback(pSwapChain, SyncInterval, Flags);
        }
    }

    // DX12 Hooks Implementation
    inline void STDMETHODCALLTYPE hkExecuteCommandLists(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists) {
        if (!g_D3D12Resources::g_pd3dCommandQueue && g_InitState::g_AfterFirstPresent && queue) {
            D3D12_COMMAND_QUEUE_DESC desc = queue->GetDesc();

            if (desc.Type == D3D12_COMMAND_LIST_TYPE_DIRECT) {
                ID3D12Device* tempDevice = nullptr;

                if (SUCCEEDED(queue->GetDevice(IID_PPV_ARGS(&tempDevice)))) {
                    tempDevice->Release();
                    queue->AddRef();
                    g_D3D12Resources::g_pd3dCommandQueue = queue;
                }
            }
        }

        if (g_HookFunctions::g_oExecuteCommandLists) {
            g_HookFunctions::g_oExecuteCommandLists(queue, NumCommandLists, ppCommandLists);
        }
    }

    inline HRESULT STDMETHODCALLTYPE hkPresent(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags) {
        g_InitState::g_AfterFirstPresent = true;

        if (!g_D3D12Resources::g_pd3dCommandQueue) {
            if (g_HookFunctions::g_oPresent) return g_HookFunctions::g_oPresent(pSwapChain, SyncInterval, Flags);
            return S_OK;
        }

        if (!g_InitState::g_Initialized) {
            std::lock_guard<std::mutex> lock(g_InitState::g_InitMutex);
            ID3D12Device* deviceFromSwap = nullptr;

            if (FAILED(pSwapChain->GetDevice(IID_PPV_ARGS(&deviceFromSwap)))) {
                if (g_HookFunctions::g_oPresent) return g_HookFunctions::g_oPresent(pSwapChain, SyncInterval, Flags);
                return S_OK;
            }

            if (!g_D3D12Resources::g_pd3dDevice) g_D3D12Resources::g_pd3dDevice = deviceFromSwap;
            else deviceFromSwap->Release();

            DXGI_SWAP_CHAIN_DESC desc{};
            pSwapChain->GetDesc(&desc);
            g_D3D12Resources::g_bufferCount = desc.BufferCount;
            HWND newWindow = desc.OutputWindow;
            GetWindowRect(newWindow, &g_ProcessWindow::g_windowRect);

            if (g_ProcessWindow::g_mainWindow != newWindow) {
                if (g_ProcessWindow::g_mainWindow) {
                    inputhook::Remove(g_ProcessWindow::g_mainWindow);
                }

                g_ProcessWindow::g_mainWindow = newWindow;
                inputhook::Init(g_ProcessWindow::g_mainWindow);
            }
            else if (!g_ProcessWindow::g_mainWindow) {
                g_ProcessWindow::g_mainWindow = newWindow;
                inputhook::Init(g_ProcessWindow::g_mainWindow);
            }

            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
            rtvHeapDesc.NumDescriptors = g_D3D12Resources::g_bufferCount;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

            if (FAILED(g_D3D12Resources::g_pd3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&g_D3D12Resources::g_pd3dRtvDescHeap)))) {
                if (g_HookFunctions::g_oPresent) return g_HookFunctions::g_oPresent(pSwapChain, SyncInterval, Flags);
                return S_OK;
            }

            D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{};
            srvHeapDesc.NumDescriptors = 1;
            srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

            if (FAILED(g_D3D12Resources::g_pd3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&g_D3D12Resources::g_pd3dSrvDescHeap)))) {
                if (g_HookFunctions::g_oPresent) return g_HookFunctions::g_oPresent(pSwapChain, SyncInterval, Flags);
                return S_OK;
            }

            g_D3D12Resources::g_FrameContexts.resize(g_D3D12Resources::g_bufferCount);
            UINT rtvIncrementSize = g_D3D12Resources::g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_D3D12Resources::g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();

            for (UINT i = 0; i < g_D3D12Resources::g_bufferCount; ++i) {
                if (FAILED(g_D3D12Resources::g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_D3D12Resources::g_FrameContexts[i].CommandAllocator)))) {
                    if (g_HookFunctions::g_oPresent) return g_HookFunctions::g_oPresent(pSwapChain, SyncInterval, Flags);
                    return S_OK;
                }

                if (FAILED(pSwapChain->GetBuffer(i, IID_PPV_ARGS(&g_D3D12Resources::g_FrameContexts[i].Resource)))) {
                    if (g_HookFunctions::g_oPresent) return g_HookFunctions::g_oPresent(pSwapChain, SyncInterval, Flags);
                    return S_OK;
                }

                g_D3D12Resources::g_FrameContexts[i].Descriptor = rtvHandle;
                g_D3D12Resources::g_pd3dDevice->CreateRenderTargetView(g_D3D12Resources::g_FrameContexts[i].Resource, nullptr, rtvHandle);
                rtvHandle.ptr += static_cast<SIZE_T>(rtvIncrementSize);
            }

            if (FAILED(g_D3D12Resources::g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_D3D12Resources::g_FrameContexts[0].CommandAllocator, nullptr, IID_PPV_ARGS(&g_D3D12Resources::g_pd3dCommandList)))) {
                if (g_HookFunctions::g_oPresent) return g_HookFunctions::g_oPresent(pSwapChain, SyncInterval, Flags);
                return S_OK;
            }

            g_D3D12Resources::g_pd3dCommandList->Close();

            if (FAILED(g_D3D12Resources::g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_D3D12Resources::g_fence)))) {
                if (g_HookFunctions::g_oPresent) return g_HookFunctions::g_oPresent(pSwapChain, SyncInterval, Flags);
                return S_OK;
            }

            g_D3D12Resources::g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

            if (!g_D3D12Resources::g_fenceEvent) {
                if (g_HookFunctions::g_oPresent) return g_HookFunctions::g_oPresent(pSwapChain, SyncInterval, Flags);
                return S_OK;
            }

            if (!ImGui::GetCurrentContext()) {
                ImGui::CreateContext();

                // 只有首次初始化才跑它
                // ImGui::StyleColorsClassic();
                // ImGui::StyleColorsLight();
                // ImGui::StyleColorsDark();

                ImGui_ImplWin32_Init(g_ProcessWindow::g_mainWindow);
            }

            ImGuiIO& io = ImGui::GetIO();
            io.IniFilename = nullptr;
            io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

            ImFontAtlas* atlas = io.Fonts;
            const ImWchar* range = atlas->GetGlyphRangesChineseFull();

            // 默认字体
            // Alibaba-PuHuiTi-Regular
            // g_MDX12::g_Alibaba_PuHuiTi_Regular = io.Fonts->AddFontFromMemoryTTF(g_Fonts::Alibaba_PuHuiTi_Regular, sizeof(g_Fonts::Alibaba_PuHuiTi_Regular), 18.0f, NULL, range);

            // Alibaba-PuHuiTi-Medium
            g_MDX12::g_Alibaba_PuHuiTi_Medium = io.Fonts->AddFontFromMemoryTTF(g_Fonts::Alibaba_PuHuiTi_Medium, sizeof(g_Fonts::Alibaba_PuHuiTi_Medium), 16.0f, NULL, range);

            // Alibaba-PuHuiTi-Bold
            // g_MDX12::g_Alibaba_PuHuiTi_Bold = io.Fonts->AddFontFromMemoryTTF(g_Fonts::Alibaba_PuHuiTi_Bold, sizeof(g_Fonts::Alibaba_PuHuiTi_Bold), 18.0f, NULL, range);

            // Alibaba-PuHuiTi-Heavy
            // g_MDX12::g_Alibaba_PuHuiTi_Heavy = io.Fonts->AddFontFromMemoryTTF(g_Fonts::Alibaba_PuHuiTi_Heavy, sizeof(g_Fonts::Alibaba_PuHuiTi_Heavy), 18.0f, NULL, range);

            // Alibaba-PuHuiTi-Light
            // g_MDX12::g_Alibaba_PuHuiTi_Light = io.Fonts->AddFontFromMemoryTTF(g_Fonts::Alibaba_PuHuiTi_Light, sizeof(g_Fonts::Alibaba_PuHuiTi_Light), 18.0f, NULL, range);

            // DX12 后端必须重新初始化，因为 resize 可能会让之前的 backend 对象失效
            ImGui_ImplDX12_Init(g_D3D12Resources::g_pd3dDevice, g_D3D12Resources::g_bufferCount, desc.BufferDesc.Format, g_D3D12Resources::g_pd3dSrvDescHeap, g_D3D12Resources::g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(), g_D3D12Resources::g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

            unsigned char* pixels;
            int width, height;

            io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
            rawinputhook::Init();
            cursorhook::Init();

            g_InitState::g_Initialized = true;
        }

        UINT backBufferIdx = pSwapChain->GetCurrentBackBufferIndex();
        g_D3D12Resources::FrameContext& frameCtx = g_D3D12Resources::g_FrameContexts[backBufferIdx];

        if (g_D3D12Resources::g_fence && frameCtx.FenceValue != 0) {
            UINT64 completed = g_D3D12Resources::g_fence->GetCompletedValue();

            if (completed < frameCtx.FenceValue) {
                g_D3D12Resources::g_fence->SetEventOnCompletion(frameCtx.FenceValue, g_D3D12Resources::g_fenceEvent);
                WaitForSingleObject(g_D3D12Resources::g_fenceEvent, g_InitState::g_waitTimeoutMs);
            }
        }

        frameCtx.CommandAllocator->Reset();
        g_D3D12Resources::g_pd3dCommandList->Reset(frameCtx.CommandAllocator, nullptr);

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = frameCtx.Resource;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        g_D3D12Resources::g_pd3dCommandList->ResourceBarrier(1, &barrier);
        g_D3D12Resources::g_pd3dCommandList->OMSetRenderTargets(1, &frameCtx.Descriptor, FALSE, nullptr);

        ID3D12DescriptorHeap* ppHeaps[] = { g_D3D12Resources::g_pd3dSrvDescHeap };
        g_D3D12Resources::g_pd3dCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

        ImGuiIO& io = ImGui::GetIO();
        g_MenuState::g_wasOpenLastFrame = g_MenuState::g_isOpen;
        inputhook::UpdateInputBlockState();
        cursorhook::UpdateCursorState();

        io.MouseDrawCursor = false;
        if (g_MenuState::g_isOpen) {
            io.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
        }
        else {
            io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        }

        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        SetupImGui(pSwapChain, SyncInterval, Flags);
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_D3D12Resources::g_pd3dCommandList);

        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        g_D3D12Resources::g_pd3dCommandList->ResourceBarrier(1, &barrier);
        g_D3D12Resources::g_pd3dCommandList->Close();

        if (g_D3D12Resources::g_pd3dCommandQueue) {
            if (g_HookFunctions::g_oExecuteCommandLists) {
                g_HookFunctions::g_oExecuteCommandLists(g_D3D12Resources::g_pd3dCommandQueue, 1, (ID3D12CommandList* const*)&g_D3D12Resources::g_pd3dCommandList);
            }

            UINT64 frameFence = ++g_D3D12Resources::g_fenceValue;
            frameCtx.FenceValue = frameFence;
            g_D3D12Resources::g_pd3dCommandQueue->Signal(g_D3D12Resources::g_fence, frameFence);
        }

        if (g_HookFunctions::g_oPresent) return g_HookFunctions::g_oPresent(pSwapChain, SyncInterval, Flags);

        return S_OK;
    }

    inline HRESULT STDMETHODCALLTYPE hkResizeBuffers(
        IDXGISwapChain* pSwapChain,
        UINT BufferCount,
        UINT Width,
        UINT Height,
        DXGI_FORMAT NewFormat,
        UINT SwapChainFlags)
    {
        std::lock_guard<std::mutex> lock(g_InitState::g_InitMutex);

        if (g_InitState::g_Initialized) {
            CleanupRenderResources_NoInput();
        }

        HRESULT hr = g_HookFunctions::g_oResizeBuffers ? g_HookFunctions::g_oResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags) : S_OK;

        g_InputState::g_blockMouseInput = false;
        g_InputState::g_blockKeyboardInput = false;

        return hr;
    }

    inline DWORD WINAPI MainThread(LPVOID lpParam) {
        // 初始化 MinHook
        MH_STATUS mhStatus = MH_Initialize();

        // 检查初始化状态，如果失败且不是已经初始化的状态，则返回
        if (mhStatus != MH_OK && mhStatus != MH_ERROR_ALREADY_INITIALIZED) {
            return 0;
        }

        if (!g_RuntimeModules::WaitAndLoad()) {
            // WaitAndLoad 是死循环直到成功，不会返回 false
            return 0;
        }

        // 此时 d3d12.dll 与 dxgi.dll 已被目标进程加载，可以安全操作
        while (true) {
            WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DefWindowProcW, 0, 0, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"TempDX12", nullptr };
            RegisterClassEx(&wc);
            HWND tempWnd = CreateWindow(wc.lpszClassName, L"Temp", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, nullptr, nullptr, wc.hInstance, nullptr);
            if (!tempWnd) { UnregisterClass(wc.lpszClassName, wc.hInstance); Sleep(1); continue; }

            ID3D12Device* tempDevice = nullptr;
            ID3D12CommandQueue* tempQueue = nullptr;
            IDXGIFactory4* factory = nullptr;
            IDXGISwapChain* tempSwapChain = nullptr;

            // 使用运行时加载的函数指针，而非编译期链接的符号
            bool ok = SUCCEEDED(g_RuntimeModules::g_pD3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&tempDevice)));

            if (ok) {
                D3D12_COMMAND_QUEUE_DESC qdesc = {};
                qdesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
                qdesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
                qdesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
                qdesc.NodeMask = 0;
                ok = SUCCEEDED(tempDevice->CreateCommandQueue(&qdesc, IID_PPV_ARGS(&tempQueue)));
            }

            if (ok) {
                // 使用运行时加载的函数指针
                ok = SUCCEEDED(g_RuntimeModules::g_pCreateDXGIFactory1(IID_PPV_ARGS(&factory)));
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
                    MH_CreateHook(swapVTable[8], reinterpret_cast<LPVOID>(hkPresent), reinterpret_cast<LPVOID*>(&g_HookFunctions::g_oPresent));
                }

                if (swapVTable && swapVTable[13]) {
                    MH_CreateHook(swapVTable[13], reinterpret_cast<LPVOID>(hkResizeBuffers), reinterpret_cast<LPVOID*>(&g_HookFunctions::g_oResizeBuffers));
                }

                if (queueVTable && queueVTable[10]) {
                    MH_CreateHook(queueVTable[10], reinterpret_cast<LPVOID>(hkExecuteCommandLists), reinterpret_cast<LPVOID*>(&g_HookFunctions::g_oExecuteCommandLists));
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

    // Public API
    inline void Initialize(LPVOID lpParam) {
        MainThread(lpParam);
    }
}