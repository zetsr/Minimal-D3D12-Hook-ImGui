#include "input.h"
#include "hook.h"
#include "render.h"

#pragma warning(push)
#pragma warning(disable: 26451)
#pragma warning(disable: 26812)
#pragma warning(disable: 6387)
#pragma warning(pop)

namespace cursorhook {
    static int g_cursorShowCount = 0;
    static HCURSOR g_lastCursor = nullptr;
    static bool g_initialized = false;
    static POINT g_lastReportedPos = { 0, 0 };
    BOOL WINAPI hkGetCursorPos(LPPOINT lpPoint) {
        if (!lpPoint) {
            if (oGetCursorPos) return oGetCursorPos(nullptr);

            return FALSE;
        }

        if (menu::isOpen && g_blockMouseInput) {
            RECT rect;
            GetWindowRect(g_mainWindow, &rect);
            lpPoint->x = rect.left + (rect.right - rect.left) / 2;
            lpPoint->y = rect.top + (rect.bottom - rect.top) / 2;
            g_lastReportedPos = *lpPoint;

            return TRUE;
        }

        BOOL result = oGetCursorPos ? oGetCursorPos(lpPoint) : FALSE;

        if (result) {
            g_lastReportedPos = *lpPoint;
        }

        return result;
    }

    BOOL WINAPI hkSetCursorPos(int X, int Y) {
        if (menu::isOpen && g_blockMouseInput) {
            return TRUE;
        }

        return oSetCursorPos ? oSetCursorPos(X, Y) : FALSE;
    }

    HCURSOR WINAPI hkSetCursor(HCURSOR hCursor) {
        if (menu::isOpen) {
            return oSetCursor ? oSetCursor(nullptr) : nullptr;
        }

        g_lastCursor = hCursor;

        return oSetCursor ? oSetCursor(hCursor) : nullptr;
    }

    int WINAPI hkShowCursor(BOOL bShow) {
        if (!menu::isOpen) {
            g_cursorShowCount += bShow ? 1 : -1;
        }

        if (menu::isOpen) {
            if (oShowCursor) {
                while (oShowCursor(FALSE) >= 0);
            }

            return -100;
        }

        return oShowCursor ? oShowCursor(bShow) : 0;
    }
    BOOL WINAPI hkClipCursor(const RECT* lpRect) {
        if (menu::isOpen) {
            return oClipCursor ? oClipCursor(nullptr) : FALSE;
        }

        return oClipCursor ? oClipCursor(lpRect) : FALSE;
    }

    BOOL WINAPI hkGetMouseMovePointsEx(UINT cbSize, LPMOUSEMOVEPOINT lppt, LPMOUSEMOVEPOINT lpptBuf, int nBufPoints, DWORD resolution) {
        if (menu::isOpen && g_blockMouseInput) {
            if (lppt) {
                memset(lppt, 0, cbSize);
            }

            return 0;
        }

        return oGetMouseMovePointsEx ? oGetMouseMovePointsEx(cbSize, lppt, lpptBuf, nBufPoints, resolution) : 0;
    }
    void UpdateCursorState() {
        if (menu::isOpen) {
            if (oShowCursor) {
                while (oShowCursor(FALSE) >= 0);
            }

            if (oClipCursor) {
                oClipCursor(nullptr);
            }
        }
        else {
            if (g_cursorShowCount >= 0) {
                if (oShowCursor) {
                    while (oShowCursor(TRUE) < 0);
                }
            }
            else {
                if (oShowCursor) {
                    while (oShowCursor(FALSE) >= 0);
                }
            }
        }
    }

    void Init() {
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
            MH_CreateHook(getCursorPosAddr, reinterpret_cast<LPVOID>(hkGetCursorPos), reinterpret_cast<LPVOID*>(&oGetCursorPos));
        }

        if (setCursorPosAddr) {
            MH_CreateHook(setCursorPosAddr, reinterpret_cast<LPVOID>(hkSetCursorPos), reinterpret_cast<LPVOID*>(&oSetCursorPos));
        }

        if (setCursorAddr) {
            MH_CreateHook(setCursorAddr, reinterpret_cast<LPVOID>(hkSetCursor), reinterpret_cast<LPVOID*>(&oSetCursor));
        }

        if (showCursorAddr) {
            MH_CreateHook(showCursorAddr, reinterpret_cast<LPVOID>(hkShowCursor), reinterpret_cast<LPVOID*>(&oShowCursor));
        }

        if (clipCursorAddr) {
            MH_CreateHook(clipCursorAddr, reinterpret_cast<LPVOID>(hkClipCursor), reinterpret_cast<LPVOID*>(&oClipCursor));
        }

        if (getMouseMovePointsExAddr) {
            MH_CreateHook(getMouseMovePointsExAddr, reinterpret_cast<LPVOID>(hkGetMouseMovePointsEx), reinterpret_cast<LPVOID*>(&oGetMouseMovePointsEx));
        }

        if (getClipCursorAddr) {
            oGetClipCursor = reinterpret_cast<PFN_GetClipCursor>(getClipCursorAddr);
        }

        if (oGetCursorPos) {
            oGetCursorPos(&g_lastReportedPos);
        }
        else {
            GetCursorPos(&g_lastReportedPos);
        }

        MH_EnableHook(MH_ALL_HOOKS);

        g_initialized = true;
    }

    void Remove() {
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

        oGetCursorPos = nullptr;
        oSetCursorPos = nullptr;
        oSetCursor = nullptr;
        oShowCursor = nullptr;
        oClipCursor = nullptr;
        oGetClipCursor = nullptr;
        oGetMouseMovePointsEx = nullptr;
        g_initialized = false;
    }
}

namespace rawinputhook {
    UINT WINAPI hkGetRawInputData(HRAWINPUT hRawInput, UINT uiCommand, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader) {
        if (menu::isOpen && g_blockMouseInput) {
            if (pcbSize) *pcbSize = 0;
            if (pData && pcbSize && *pcbSize > 0) {
                memset(pData, 0, *pcbSize);
            }

            return 0;
        }

        return oGetRawInputData ? oGetRawInputData(hRawInput, uiCommand, pData, pcbSize, cbSizeHeader) : 0;
    }

    UINT WINAPI hkGetRawInputBuffer(PRAWINPUT pData, PUINT pcbSize, UINT cbSizeHeader) {
        if (menu::isOpen && g_blockMouseInput) {
            if (pcbSize) {
                *pcbSize = 0;
            }

            if (pData && pcbSize && *pcbSize > 0) {
                memset(pData, 0, *pcbSize);
            }

            return 0;
        }

        return oGetRawInputBuffer ? oGetRawInputBuffer(pData, pcbSize, cbSizeHeader) : 0;
    }

    void Init() {
        HMODULE user32 = GetModuleHandleA("user32.dll");
        if (!user32) return;

        FARPROC getRawInputDataAddr = GetProcAddress(user32, "GetRawInputData");
        FARPROC getRawInputBufferAddr = GetProcAddress(user32, "GetRawInputBuffer");

        if (getRawInputDataAddr) {
            MH_CreateHook(getRawInputDataAddr, reinterpret_cast<LPVOID>(hkGetRawInputData), reinterpret_cast<LPVOID*>(&oGetRawInputData));
        }

        if (getRawInputBufferAddr) {
            MH_CreateHook(getRawInputBufferAddr, reinterpret_cast<LPVOID>(hkGetRawInputBuffer), reinterpret_cast<LPVOID*>(&oGetRawInputBuffer));
        }

        if (getRawInputDataAddr) MH_EnableHook(getRawInputDataAddr);
        if (getRawInputBufferAddr) MH_EnableHook(getRawInputBufferAddr);
    }

    void Remove() {
        HMODULE user32 = GetModuleHandleA("user32.dll");
        if (!user32) return;

        FARPROC getRawInputDataAddr = GetProcAddress(user32, "GetRawInputData");
        FARPROC getRawInputBufferAddr = GetProcAddress(user32, "GetRawInputBuffer");

        if (getRawInputDataAddr && oGetRawInputData) {
            MH_DisableHook(getRawInputDataAddr);
            MH_RemoveHook(getRawInputDataAddr);
        }

        if (getRawInputBufferAddr && oGetRawInputBuffer) {
            MH_DisableHook(getRawInputBufferAddr);
            MH_RemoveHook(getRawInputBufferAddr);
        }

        oGetRawInputData = nullptr;
        oGetRawInputBuffer = nullptr;
    }
}

namespace inputhook {
    static WNDPROC sOriginalWndProc = nullptr;
    static bool g_f1Down = false;

    void UpdateInputBlockState() {
        HWND foreground = GetForegroundWindow();
        bool isGameForeground = (foreground == g_mainWindow);
        char className[256] = { 0 };

        if (foreground && foreground != g_mainWindow) {
            GetClassNameA(foreground, className, sizeof(className));
        }

        std::unordered_set<std::string> menuClasses = {
        "#32770",
        "ConsoleWindowClass",
        "Edit",
        "ListBox",
        };

        bool gameHasMenuOpen = false;

        if (foreground && foreground != g_mainWindow) {
            DWORD pid;
            GetWindowThreadProcessId(foreground, &pid);
            DWORD gamePid = GetCurrentProcessId();

            if (pid == gamePid) {
                gameHasMenuOpen = true;
            }
        }

        g_blockMouseInput = (menu::isOpen && !gameHasMenuOpen);
    }

    LRESULT APIENTRY WndProcHook(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        if (uMsg == WM_KEYDOWN && wParam == VK_F1 && !g_f1Down) {
            g_f1Down = true;
            menu::isOpen = !menu::isOpen;

            if (menu::isOpen) {
                GetCursorPos(&menu::s_lastMousePos);
                RECT rect;
                GetWindowRect(g_mainWindow, &rect);
                int centerX = rect.left + (rect.right - rect.left) / 2;
                int centerY = rect.top + (rect.bottom - rect.top) / 2;
                SetCursorPos(centerX, centerY);
            }
            else {
                SetCursorPos(menu::s_lastMousePos.x, menu::s_lastMousePos.y);
            }

            return 0;
        }

        else if (uMsg == WM_KEYUP && wParam == VK_F1) {
            g_f1Down = false;
            return 0;
        }

        if (uMsg == WM_INPUT) {
            if (menu::isOpen && g_blockMouseInput) {
                return 0;
            }
        }

        if (uMsg == WM_INPUT_DEVICE_CHANGE) {
            if (menu::isOpen && g_blockMouseInput) {
                return 0;
            }
        }

        if (menu::isOpen) {
            ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);
            ImGuiIO& io = ImGui::GetIO();

            if (g_blockMouseInput) {
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

    void Init(HWND hWindow) {
        if (!hWindow) return;
        g_mainWindow = hWindow;

        if (sOriginalWndProc) {
            Remove(hWindow);
        }

        sOriginalWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProcHook)));
    }

    void Remove(HWND hWindow) {
        if (!hWindow || !sOriginalWndProc) return;

        SetWindowLongPtr(hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(sOriginalWndProc));
        sOriginalWndProc = nullptr;
    }

    void ReinstallWindowHook() {
        if (!g_mainWindow) return;

        if (!sOriginalWndProc) {
            Init(g_mainWindow);
        }
    }
}
