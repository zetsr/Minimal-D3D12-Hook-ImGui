#include "input.h"
#include "hook.h"
#include "render.h"

#pragma warning(push)
#pragma warning(disable: 26451)
#pragma warning(disable: 26812)
#pragma warning(disable: 6387)
#pragma warning(pop)

// 下面是你原始代码中关于 input / rawinput / cursor 的全部实现，已原样迁移到这里

// cursorhook
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
        // 菜单打开且拦截鼠标时，返回窗口中心
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
        // 菜单打开且拦截鼠标时，阻止设置光标位置
        if (menu::isOpen && g_blockMouseInput) {
            return TRUE;
        }
        return oSetCursorPos ? oSetCursorPos(X, Y) : FALSE;
    }
    HCURSOR WINAPI hkSetCursor(HCURSOR hCursor) {
        // 菜单打开时，隐藏光标
        if (menu::isOpen) {
            return oSetCursor ? oSetCursor(nullptr) : nullptr;
        }
        // 菜单关闭时，允许游戏设置光标
        g_lastCursor = hCursor;
        return oSetCursor ? oSetCursor(hCursor) : nullptr;
    }
    int WINAPI hkShowCursor(BOOL bShow) {
        // 跟踪游戏光标显示状态（仅在菜单关闭时）
        if (!menu::isOpen) {
            g_cursorShowCount += bShow ? 1 : -1;
        }
        // 菜单打开时，强制隐藏光标
        if (menu::isOpen) {
            if (oShowCursor) {
                while (oShowCursor(FALSE) >= 0);
            }
            return -100;
        }
        return oShowCursor ? oShowCursor(bShow) : 0;
    }
    BOOL WINAPI hkClipCursor(const RECT* lpRect) {
        // 菜单打开时，解除光标限制
        if (menu::isOpen) {
            return oClipCursor ? oClipCursor(nullptr) : FALSE;
        }
        return oClipCursor ? oClipCursor(lpRect) : FALSE;
    }
    BOOL WINAPI hkGetMouseMovePointsEx(UINT cbSize, LPMOUSEMOVEPOINT lppt, LPMOUSEMOVEPOINT lpptBuf, int nBufPoints, DWORD resolution) {
        // 菜单打开且拦截鼠标时，返回没有移动点
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
            // 菜单打开时：强制隐藏系统光标
            if (oShowCursor) {
                while (oShowCursor(FALSE) >= 0);
            }
            if (oClipCursor) {
                oClipCursor(nullptr);
            }
        }
        else {
            // 菜单关闭时：恢复游戏的光标状态
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

// rawinputhook implementation
namespace rawinputhook {
    UINT WINAPI hkGetRawInputData(HRAWINPUT hRawInput, UINT uiCommand, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader) {
        // 仅当需要拦截鼠标且菜单打开时拦截
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

// inputhook (WndProc) implementation
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
        // 关键修复：只有当菜单打开时才拦截
        // 鼠标：始终拦截（如果菜单打开且游戏没有自己的菜单）
        // 键盘：只拦截 ImGui 需要的
        g_blockMouseInput = (menu::isOpen && !gameHasMenuOpen);
    }
    LRESULT APIENTRY WndProcHook(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        // F1 切换菜单
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
        // 处理 WM_INPUT 消息
        if (uMsg == WM_INPUT) {
            if (menu::isOpen && g_blockMouseInput) {
                return 0;
            }
        }
        // 处理 WM_INPUT_DEVICE_CHANGE
        if (uMsg == WM_INPUT_DEVICE_CHANGE) {
            if (menu::isOpen && g_blockMouseInput) {
                return 0;
            }
        }
        // 菜单打开时处理输入
        if (menu::isOpen) {
            // 先让 ImGui 处理输入（这不会消耗消息）
            ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);
            ImGuiIO& io = ImGui::GetIO();
            // 鼠标消息：总是拦截（因为我们需要限制鼠标在中心）
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
            // 键盘消息：仅当 ImGui 实际想要捕获时才拦截
            // 这样可以让游戏接收WASD等按键用于移动
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
        // 所有其他消息传递给原始窗口过程
        if (sOriginalWndProc) {
            return CallWindowProc(sOriginalWndProc, hwnd, uMsg, wParam, lParam);
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    void Init(HWND hWindow) {
        if (!hWindow) return;
        g_mainWindow = hWindow;
        // 检查是否已经安装了钩子
        if (sOriginalWndProc) {
            // 如果已经安装，先移除再重新安装
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
        // 如果窗口句柄有效但窗口钩子丢失，重新安装
        if (!sOriginalWndProc) {
            Init(g_mainWindow);
        }
    }
}
