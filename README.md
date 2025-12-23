# Minimal-D3D12-Hook-ImGui

一个轻量级的 DirectX 12 图形界面注入库，通过 ImGui 实现图形绘制。

该项目采用最小化设计理念，封装了复杂的 D3D12 Hook 过程，使开发者可以专注于 UI 开发。

## 项目特性

- **开箱即用** - 仅需包含一个头文件，通过简单的回调函数即可实现自定义 UI 绘制
- **结构清晰** - 所有全局变量按功能分类管理，避免命名空间污染
- **兼容性强** - 支持 DirectX 12 应用程序的自动检测和 Hook
- **轻量高效** - 最小化的依赖和代码体积，不影响宿主程序性能
- **输入处理** - 完整的鼠标和键盘输入 Hook 机制，支持灵活的交互控制

## 快速开始

### 1. 项目配置

在项目中包含必要的头文件和库：

```cpp
#include "mdx12_api.h"
```

确保以下文件包含在你的项目中：
- `mdx12_api.h` - 主 API 头文件
- `mdx12_globals.cpp` - 全局变量定义
- `hook.cpp` - D3D12 Hook 实现
- `input.cpp` - 输入处理
- `render.cpp` - 渲染资源管理
- `setup_imgui.cpp` - ImGui 集成

### 2. 基本使用

在 `dllmain.cpp` 中编写你的代码：

```cpp
#include "mdx12_api.h"

// 定义自定义 ImGui 绘制函数
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
            
            ImGui::End();
        }
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        // 初始化 Hook 系统
        g_MDX12::Initialize();
        
        // 设置自定义绘制回调
        g_MDX12::SetSetupImGuiCallback(MyImGuiDraw);
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

## API 参考

### 全局状态访问

#### 菜单状态
```cpp
g_MDX12::g_MenuState::g_isOpen              // 菜单当前是否显示
g_MDX12::g_MenuState::g_wasOpenLastFrame    // 上一帧的菜单状态
g_MDX12::g_MenuState::g_lastMousePos        // 上一次记录的鼠标位置
```

#### 输入状态
```cpp
g_MDX12::g_InputState::g_blockMouseInput      // 是否阻挡鼠标输入
g_MDX12::g_InputState::g_blockKeyboardInput   // 是否阻挡键盘输入
```

#### 窗口信息
```cpp
g_MDX12::g_ProcessWindow::g_mainWindow  // 主窗口句柄
g_MDX12::g_ProcessWindow::g_windowRect  // 主窗口矩形区域
```

#### D3D12 资源
```cpp
g_MDX12::g_D3D12Resources::g_pd3dDevice        // D3D12 设备
g_MDX12::g_D3D12Resources::g_pd3dCommandQueue  // 命令队列
g_MDX12::g_D3D12Resources::g_FrameContexts     // 帧缓冲上下文
```

### 公开函数

#### 初始化和清理
```cpp
void g_MDX12::Initialize()           // 启动 Hook 系统
void g_MDX12::FinalCleanupAll()      // 清理所有资源
```

#### 回调设置
```cpp
void g_MDX12::SetSetupImGuiCallback(SetupImGuiCallback callback)
// 设置自定义的 ImGui 绘制回调函数
```

#### 导出的 C 函数
```cpp
void SetOverlayWaitTimeout(UINT ms)           // 设置同步超时时间（毫秒）
void SetSetupImGuiCallback(SetupImGuiCallback) // 设置 ImGui 绘制回调
```

## 架构设计

该项目采用模块化设计，所有全局变量统一管理在 `g_MDX12` 命名空间下：

```
g_MDX12/
├── g_HookFunctions/      - Hook 函数指针存储
├── g_D3D12Resources/     - Direct3D 12 资源管理
├── g_InitState/          - 初始化和状态标志
├── g_ProcessWindow/      - 进程窗口信息
├── g_InputState/         - 输入状态管理
└── g_MenuState/          - 菜单相关状态
```

所有全局变量均以 `g_` 前缀开头，确保了命名空间的清晰性。

## 文件说明

| 文件 | 说明 |
|------|------|
| `mdx12_api.h` | 主 API 头文件，包含所有公开接口 |
| `mdx12_globals.cpp` | 全局变量定义和初始化 |
| `hook.cpp` | D3D12 Present 和 ResizeBuffers 的 Hook 实现 |
| `input.cpp` | 鼠标、键盘和原始输入的 Hook 和管理 |
| `render.cpp` | 渲染资源生命周期管理 |
| `setup_imgui.cpp` | ImGui 绘制逻辑和回调处理 |
| `dllmain.cpp` | DLL 入口点和应用示例 |

## 技术细节

### Hook 机制

项目通过以下步骤实现 D3D12 Hook：

1. 在后台线程中创建临时 D3D12 设备和交换链
2. 提取虚表地址并使用 MinHook 创建 Hook
3. 在 Present 调用时拦截并绘制 ImGui 内容
4. 自动处理缓冲区重置和资源管理

### 输入处理

- Window 消息截获：通过 WndProc Hook 捕获窗口消息
- 原始输入 Hook：GetRawInputData 和 GetRawInputBuffer
- 光标控制：SetCursorPos、GetCursorPos 等 Hook
- 智能拦截：根据菜单状态自动阻挡或转发输入

### 线程安全

使用互斥锁保护初始化过程，确保在多线程环境下的资源一致性。

## 兼容性

- **操作系统**：Windows 10/11 及更新版本
- **Visual Studio**：2019 或更新版本
- **C++ 标准**：C++14 及以上
- **DirectX**：支持 DirectX 12

## 配置选项

### 修改菜单热键

在 `input.cpp` 的 `WndProcHook` 函数中修改：

```cpp
if (uMsg == WM_KEYDOWN && wParam == VK_F1 && !g_f1Down) {
    // 改为其他虚拟键码，如 VK_INSERT
}
```

### 调整超时时间

```cpp
SetOverlayWaitTimeout(3000);  // 设置为 3000 毫秒
```

## 常见问题

**Q: 可以改变菜单的热键吗？**  
A: 可以。在 `input.cpp` 中修改 `WndProcHook` 函数中的键码判断。

**Q: 如何自定义菜单样式？**  
A: 在你的绘制回调函数中使用 ImGui 的样式 API，例如 `ImGui::StyleColorsDark()` 或自定义颜色。

**Q: 支持多个窗口吗？**  
A: 当前实现针对单个主窗口设计。多窗口支持需要修改窗口检测逻辑。

**Q: 项目可以用于商业用途吗？**  
A: 可以，但需遵守 ImGui、MinHook 等依赖库的许可证要求。

## 致谢

感谢以下项目的支持：

- [ImGui](https://github.com/ocornut/imgui) - 即时模式图形用户界面库
- [MinHook](https://github.com/TsudaKageyu/minhook) - 轻量级 API Hook 库
- [Universal-Dear-ImGui-Hook](https://github.com/Sh0ckFR/Universal-Dear-ImGui-Hook) - ImGui Hook 参考实现

> 最后，感谢 *Large Language Model*，它帮助我实现了无数个在此之前难以实现的梦想：
>
> **做更多有用的东西，让更多人使用。**

## 许可证

该项目遵循其依赖库的许可证。详见各库的许可证文件。
