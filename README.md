# Minimal-D3D12-Hook-ImGui

一个轻量级的 DirectX 12 钩子库，仅需包含一个头文件即可通过回调函数运行 ImGui。

<img width="1919" height="998" alt="image" src="https://github.com/user-attachments/assets/33e23a19-bc52-4027-8059-3c3af84a4cd9" />

## 快速开始

### 1. 项目配置

在项目中包含头文件：

```cpp
#include "mdx12_api.h"
```
### 2. 开始使用

在 `dllmain.cpp` 中编写你的业务代码：

```cpp
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
```

### 3. 菜单切换

默认使用 **F1** 键来切换菜单的显示/隐藏。菜单打开时，鼠标和键盘输入将被自动转发给 ImGui。

## 致谢

- [Dear ImGui](https://github.com/ocornut/imgui)
- [MinHook](https://github.com/TsudaKageyu/minhook)
- [Universal-Dear-ImGui-Hook](https://github.com/Sh0ckFR/Universal-Dear-ImGui-Hook)

> 最后，感谢 *Large Language Model*，它帮助我实现了无数个在此之前难以实现的梦想：
>
> **做更多有用的东西，让更多人使用。**
