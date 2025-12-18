#include "render.h"
#include "input.h"

#pragma warning(push)
#pragma warning(disable: 26451)
#pragma warning(disable: 26812)
#pragma warning(disable: 6387)
#pragma warning(pop)

// 获取进程名
void InitProcessName() {
    char path[MAX_PATH] = { 0 };
    if (GetModuleFileNameA(nullptr, path, MAX_PATH) > 0) {
        std::string s(path);
        size_t pos = s.find_last_of("\/");
        if (pos != std::string::npos) s = s.substr(pos + 1);
        g_processName = s;
    }
    else {
        g_processName = "UnknownProcess";
    }
}

// Cleanup
void CleanupRenderResources() {
    if (g_pd3dCommandQueue && g_fence && g_fenceEvent) {
        UINT64 localFence = ++g_fenceValue;
        HRESULT hr = g_pd3dCommandQueue->Signal(g_fence, localFence);
        if (SUCCEEDED(hr)) {
            if (g_fence->GetCompletedValue() < localFence) {
                g_fence->SetEventOnCompletion(localFence, g_fenceEvent);
                WaitForSingleObject(g_fenceEvent, g_waitTimeoutMs);
            }
        }
    }
    g_Initialized = false;
    if (ImGui::GetCurrentContext()) {
        ImGui_ImplDX12_InvalidateDeviceObjects();
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
    if (g_pd3dCommandList) { g_pd3dCommandList->Release(); g_pd3dCommandList = nullptr; }
    if (g_pd3dRtvDescHeap) { g_pd3dRtvDescHeap->Release(); g_pd3dRtvDescHeap = nullptr; }
    if (g_pd3dSrvDescHeap) { g_pd3dSrvDescHeap->Release(); g_pd3dSrvDescHeap = nullptr; }
    if (g_fence) { g_fence->Release(); g_fence = nullptr; }
    if (g_fenceEvent) { CloseHandle(g_fenceEvent); g_fenceEvent = nullptr; }
    for (auto& frame : g_FrameContexts) {
        if (frame.Resource) { frame.Resource->Release(); frame.Resource = nullptr; }
        if (frame.CommandAllocator) { frame.CommandAllocator->Release(); frame.CommandAllocator = nullptr; }
        frame.FenceValue = 0;
    }
    g_FrameContexts.clear();
    g_bufferCount = 0;
    // 注意：这里不删除窗口钩子，因为ResizeBuffers后窗口句柄不变
    // 我们只在最终清理时才删除窗口钩子
    rawinputhook::Remove();
    cursorhook::Remove();
}

// ======================== 修复关键新增 ========================
void CleanupRenderResources_NoInput() {
    if (g_pd3dCommandQueue && g_fence && g_fenceEvent) {
        UINT64 localFence = ++g_fenceValue;
        if (SUCCEEDED(g_pd3dCommandQueue->Signal(g_fence, localFence))) {
            if (g_fence->GetCompletedValue() < localFence) {
                g_fence->SetEventOnCompletion(localFence, g_fenceEvent);
                WaitForSingleObject(g_fenceEvent, g_waitTimeoutMs);
            }
        }
    }
    if (ImGui::GetCurrentContext()) {
        ImGui_ImplDX12_InvalidateDeviceObjects();
    }
    if (g_pd3dCommandList) { g_pd3dCommandList->Release(); g_pd3dCommandList = nullptr; }
    if (g_pd3dRtvDescHeap) { g_pd3dRtvDescHeap->Release(); g_pd3dRtvDescHeap = nullptr; }
    if (g_pd3dSrvDescHeap) { g_pd3dSrvDescHeap->Release(); g_pd3dSrvDescHeap = nullptr; }
    for (auto& frame : g_FrameContexts) {
        if (frame.Resource) { frame.Resource->Release(); frame.Resource = nullptr; }
        if (frame.CommandAllocator) { frame.CommandAllocator->Release(); frame.CommandAllocator = nullptr; }
        frame.FenceValue = 0;
    }
    g_FrameContexts.clear();
    g_bufferCount = 0;
    g_Initialized = false;
}

void FinalCleanupAll() {
    std::lock_guard<std::mutex> lock(g_InitMutex);
    if (g_Initialized) CleanupRenderResources();
    if (g_pd3dCommandQueue && g_fence && g_fenceEvent) {
        UINT64 localFence = ++g_fenceValue;
        g_pd3dCommandQueue->Signal(g_fence, localFence);
        if (g_fence->GetCompletedValue() < localFence) {
            g_fence->SetEventOnCompletion(localFence, g_fenceEvent);
            WaitForSingleObject(g_fenceEvent, INFINITE);
        }
    }
    // 最终清理时删除窗口钩子
    if (g_mainWindow) inputhook::Remove(g_mainWindow);
    if (g_pd3dCommandQueue) { g_pd3dCommandQueue->Release(); g_pd3dCommandQueue = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}
