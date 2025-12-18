#include "global.h"
#include "hook.h"
#include "render.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);

        if (CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr) == nullptr) {
            return FALSE;
        }
        break;

    case DLL_PROCESS_DETACH:
        if (lpReserved == nullptr) {
            FinalCleanupAll();
            MH_Uninitialize();
        }
        break;

    }

    return TRUE;
}