#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <vector>
#include <mutex>
#include <string>
#include <atomic>
#include <unordered_set>

#pragma warning(disable: 26451)
#pragma warning(disable: 26812)
#pragma warning(disable: 6387)
#pragma warning(push)
#pragma warning(disable: 26451)
#pragma warning(disable: 26812)
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx12.h"
#include "MinHook/include/MinHook.h"
#pragma warning(pop)

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "user32.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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

inline PFN_Present oPresent = nullptr;
inline PFN_ExecuteCommandLists oExecuteCommandLists = nullptr;
inline PFN_ResizeBuffers oResizeBuffers = nullptr;
inline PFN_GetRawInputData oGetRawInputData = nullptr;
inline PFN_GetRawInputBuffer oGetRawInputBuffer = nullptr;
inline PFN_GetCursorPos oGetCursorPos = nullptr;
inline PFN_SetCursorPos oSetCursorPos = nullptr;
inline PFN_SetCursor oSetCursor = nullptr;
inline PFN_ShowCursor oShowCursor = nullptr;
inline PFN_GetClipCursor oGetClipCursor = nullptr;
inline PFN_ClipCursor oClipCursor = nullptr;
inline PFN_GetMouseMovePointsEx oGetMouseMovePointsEx = nullptr;

inline ID3D12Device* g_pd3dDevice = nullptr;
inline ID3D12CommandQueue* g_pd3dCommandQueue = nullptr;
inline ID3D12DescriptorHeap* g_pd3dRtvDescHeap = nullptr;
inline ID3D12DescriptorHeap* g_pd3dSrvDescHeap = nullptr;
inline ID3D12GraphicsCommandList* g_pd3dCommandList = nullptr;
inline ID3D12Fence* g_fence = nullptr;
inline HANDLE g_fenceEvent = nullptr;
inline UINT64 g_fenceValue = 0;
inline UINT g_bufferCount = 0;
struct FrameContext {
    ID3D12CommandAllocator* CommandAllocator = nullptr;
    ID3D12Resource* Resource = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE Descriptor{};
    UINT64 FenceValue = 0;
};
inline std::vector<FrameContext> g_FrameContexts;
inline bool g_Initialized = false;
inline bool g_AfterFirstPresent = false;
inline std::mutex g_InitMutex;
inline UINT g_waitTimeoutMs = 2000;

extern "C" __declspec(dllexport) inline void SetOverlayWaitTimeout(UINT ms) { g_waitTimeoutMs = ms; }

inline std::string g_processName;

namespace menu {
    inline bool isOpen = true;
    inline bool wasOpenLastFrame = true;
    inline POINT s_lastMousePos = { 0, 0 };
}

inline HWND g_mainWindow = nullptr;
inline RECT g_windowRect = { 0 };
inline std::atomic<bool> g_blockMouseInput{ false };
inline std::atomic<bool> g_blockKeyboardInput{ false };

namespace inputhook {
    void Init(HWND hWindow);
    void Remove(HWND hWindow);
    void UpdateInputBlockState();
    void ReinstallWindowHook();
}

namespace rawinputhook {
    void Init();
    void Remove();
}

namespace cursorhook {
    void Init();
    void Remove();
    void UpdateCursorState();
}

void InitProcessName();
void CleanupRenderResources();
void CleanupRenderResources_NoInput();
void FinalCleanupAll();

DWORD WINAPI MainThread(LPVOID);