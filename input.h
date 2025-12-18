#pragma once
#include "global.h"

namespace inputhook {
    void Init(HWND hWindow);
    void Remove(HWND hWindow);
    void UpdateInputBlockState();
    void ReinstallWindowHook();
}

namespace rawinputhook {
    UINT WINAPI hkGetRawInputData(HRAWINPUT hRawInput, UINT uiCommand, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader);
    UINT WINAPI hkGetRawInputBuffer(PRAWINPUT pData, PUINT pcbSize, UINT cbSizeHeader);
    void Init();
    void Remove();
}

namespace cursorhook {
    void Init();
    void Remove();
    void UpdateCursorState();
}