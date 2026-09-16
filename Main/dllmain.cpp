#include <windows.h>
#include "mdx12_api.h"

void MyImGuiDraw(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags) {
    if (g_MDX12::g_MenuState::g_isOpen) {
		ImGui::ShowDemoWindow(nullptr);
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
    }
    return TRUE;
}