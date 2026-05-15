# Minimal-D3D12-Hook-ImGui

一个轻量级的 DirectX 12 钩子库，仅需包含一个头文件即可通过简单的回调函数运行 ImGui。

## 快速开始

### 1. 项目配置

在项目中包含必要的头文件和库：

```cpp
#include "mdx12_api.h"
```

确保以下文件包含在你的项目中：
- `mdx12_api.h` - 主 API 头文件
- `mdx12_globals.cpp` - 全局变量定义
- `hooks.cpp` - D3D12 Hook 实现
- `input.cpp` - 输入处理
- `render.cpp` - 渲染资源管理
- `setup_imgui.cpp` - ImGui 集成

### 2. 基本使用

在 `dllmain.cpp` 中编写你的代码：

```cpp
#include "mdx12_api.h"

/// 定义自定义 ImGui 绘制函数
void MyImGuiDraw(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{
    // 检查菜单是否打开（按 F1 切换）
    if (g_MDX12::g_MenuState::g_isOpen) {
        ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);

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
```

### 3. 菜单切换

默认使用 **F1** 键来切换菜单的显示/隐藏。菜单打开时，鼠标和键盘输入将被自动转发给 ImGui。

## 致谢

- [ImGui](https://github.com/ocornut/imgui)
- [MinHook](https://github.com/TsudaKageyu/minhook)
- [Universal-Dear-ImGui-Hook](https://github.com/Sh0ckFR/Universal-Dear-ImGui-Hook)

> 最后，感谢 *Large Language Model*，它帮助我实现了无数个在此之前难以实现的梦想：
>
> **做更多有用的东西，让更多人使用。**

## 许可证

该项目遵循其依赖库的许可证。详见各库的许可证文件。