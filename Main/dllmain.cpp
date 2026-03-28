#include "mdx12_api.h"

extern "C" {
#include "../MinHook/src/buffer.c"
#include "../MinHook/src/hook.c"
#include "../MinHook/src/trampoline.c"
#include "../MinHook/src/hde/hde32.c"
#include "../MinHook/src/hde/hde64.c"
}

/// 定义自定义 ImGui 绘制函数
void MyImGuiDraw(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{
    // 检查菜单是否打开（按 F1 切换）
    if (g_MDX12::g_MenuState::g_isOpen) {
        ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);

        // 注意：即使 Begin 返回 false（窗口被折叠），你依然需要调用 End()
        if (ImGui::Begin("My Menu")) {
            ImGui::Text("Hello World!");

            static bool option = false;
            ImGui::Checkbox("My Option", &option);
        }
        ImGui::End();
    }
}

void init(LPVOID lpParam) {
    g_MDX12::Initialize(lpParam);
    g_MDX12::SetSetupImGuiCallback(MyImGuiDraw);
}

void MainThread(LPVOID lpParam) {
    init(lpParam);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        if (HANDLE h = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr)) CloseHandle(h);
        break;

    case DLL_PROCESS_DETACH:
        // 清理资源
        g_MDX12::FinalCleanupAll();
        break;
    }
    return TRUE;
}