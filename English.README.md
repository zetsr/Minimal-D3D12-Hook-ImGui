# Minimal-D3D12-Hook-ImGui

A lightweight Direct3D 12 graphical interface injection library providing fast UI rendering through ImGui.

This project adopts a minimalist design philosophy, encapsulating the complexity of D3D12 Hook procedures, allowing developers to focus on UI logic implementation.

## Project Features

- **Ready Out of the Box** - Include only a single header file and implement custom UI rendering through simple callback functions
- **Clear Architecture** - All global variables are categorized and managed by functionality, preventing namespace pollution
- **Strong Compatibility** - Supports automatic detection and Hook of Direct3D 12 applications
- **Lightweight and Efficient** - Minimal dependencies and code footprint with no performance impact on host applications
- **Input Handling** - Complete mouse and keyboard input Hook mechanism with flexible interaction control

## Quick Start

### 1. Project Configuration

Include the necessary header file in your project:

```cpp
#include "mdx12_api.h"
```

Ensure the following files are included in your project:
- `mdx12_api.h` - Main API header file
- `mdx12_globals.cpp` - Global variable definitions
- `hook.cpp` - Direct3D 12 Hook implementation
- `input.cpp` - Input processing
- `render.cpp` - Render resource management
- `setup_imgui.cpp` - ImGui integration

### 2. Basic Usage

Write your code in `dllmain.cpp`:

```cpp
#include "mdx12_api.h"

// Define your custom ImGui drawing function
void MyImGuiDraw(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{
    // Check if menu is open (toggle with F1)
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
        // Initialize Hook system
        g_MDX12::Initialize();
        
        // Set custom drawing callback
        g_MDX12::SetSetupImGuiCallback(MyImGuiDraw);
        break;
        
    case DLL_PROCESS_DETACH:
        // Clean up resources
        g_MDX12::FinalCleanupAll();
        break;
    }
    return TRUE;
}
```

### 3. Menu Toggle

By default, the **F1** key toggles menu visibility. When the menu is open, mouse and keyboard input are automatically forwarded to ImGui.

## API Reference

### Global State Access

#### Menu State
```cpp
g_MDX12::g_MenuState::g_isOpen              // Whether menu is currently visible
g_MDX12::g_MenuState::g_wasOpenLastFrame    // Menu state from previous frame
g_MDX12::g_MenuState::g_lastMousePos        // Last recorded mouse position
```

#### Input State
```cpp
g_MDX12::g_InputState::g_blockMouseInput      // Whether to block mouse input
g_MDX12::g_InputState::g_blockKeyboardInput   // Whether to block keyboard input
```

#### Window Information
```cpp
g_MDX12::g_ProcessWindow::g_mainWindow  // Main window handle
g_MDX12::g_ProcessWindow::g_windowRect  // Main window rectangle
```

#### Direct3D 12 Resources
```cpp
g_MDX12::g_D3D12Resources::g_pd3dDevice        // D3D12 device
g_MDX12::g_D3D12Resources::g_pd3dCommandQueue  // Command queue
g_MDX12::g_D3D12Resources::g_FrameContexts     // Frame buffer contexts
```

### Public Functions

#### Initialization and Cleanup
```cpp
void g_MDX12::Initialize()           // Start Hook system
void g_MDX12::FinalCleanupAll()      // Clean up all resources
```

#### Callback Setup
```cpp
void g_MDX12::SetSetupImGuiCallback(SetupImGuiCallback callback)
// Set custom ImGui drawing callback function
```

#### Exported C Functions
```cpp
void SetOverlayWaitTimeout(UINT ms)           // Set synchronization timeout (milliseconds)
void SetSetupImGuiCallback(SetupImGuiCallback) // Set ImGui drawing callback
```

## Architecture Design

The project employs modular design with all global variables unified under the `g_MDX12` namespace:

```
g_MDX12/
├── g_HookFunctions/      - Hook function pointer storage
├── g_D3D12Resources/     - Direct3D 12 resource management
├── g_InitState/          - Initialization and state flags
├── g_ProcessWindow/      - Process window information
├── g_InputState/         - Input state management
└── g_MenuState/          - Menu-related state
```

All global variables are prefixed with `g_`, ensuring clear namespace organization.

## File Reference

| File | Description |
|------|-------------|
| `mdx12_api.h` | Main API header file containing all public interfaces |
| `mdx12_globals.cpp` | Global variable definitions and initialization |
| `hook.cpp` | Direct3D 12 Present and ResizeBuffers Hook implementation |
| `input.cpp` | Mouse, keyboard, and raw input Hook and management |
| `render.cpp` | Render resource lifecycle management |
| `setup_imgui.cpp` | ImGui drawing logic and callback handling |
| `dllmain.cpp` | DLL entry point and usage example |

## Technical Details

### Hook Mechanism

The project implements D3D12 Hook through the following steps:

1. Create temporary D3D12 device and swap chain in background thread
2. Extract virtual table address and create Hook using MinHook
3. Intercept and render ImGui content during Present call
4. Automatically handle buffer reset and resource management

### Input Processing

- Window Message Interception: Capture window messages through WndProc Hook
- Raw Input Hook: GetRawInputData and GetRawInputBuffer Hook
- Cursor Control: Hook for SetCursorPos, GetCursorPos, etc.
- Smart Interception: Automatically block or forward input based on menu state

### Thread Safety

Uses mutex locks to protect initialization process, ensuring resource consistency in multi-threaded environments.

## Compatibility

- **Operating System**: Windows 10/11 and newer
- **Visual Studio**: 2019 or newer
- **C++ Standard**: C++14 or higher
- **DirectX**: Direct3D 12 support

## Configuration Options

### Change Menu Hotkey

In the `WndProcHook` function in `input.cpp`, modify:

```cpp
if (uMsg == WM_KEYDOWN && wParam == VK_F1 && !g_f1Down) {
    // Change to another virtual key code, e.g., VK_INSERT
}
```

### Adjust Timeout Value

```cpp
SetOverlayWaitTimeout(3000);  // Set to 3000 milliseconds
```

## Frequently Asked Questions

**Q: Can I change the menu hotkey?**  
A: Yes. Modify the key code check in the `WndProcHook` function in `input.cpp`.

**Q: How do I customize the menu style?**  
A: Use ImGui's style API in your drawing callback function, such as `ImGui::StyleColorsDark()` or custom colors.

**Q: Does it support multiple windows?**  
A: The current implementation is designed for a single main window. Multi-window support requires modifications to window detection logic.

**Q: Can this project be used for commercial purposes?**  
A: Yes, but you must comply with the license requirements of dependency libraries such as ImGui and MinHook.

## Credits

Special thanks to the following projects:

- [ImGui](https://github.com/ocornut/imgui) - Immediate-mode graphical user interface library
- [MinHook](https://github.com/TsudaKageyu/minhook) - Lightweight API Hook library
- [Universal-Dear-ImGui-Hook](https://github.com/Sh0ckFR/Universal-Dear-ImGui-Hook) - ImGui Hook reference implementation

> **I would like to extend my gratitude to *Large Language Models*, which have helped me realize countless dreams that were previously difficult to achieve: creating more useful things and allowing more people to use them.**

## License

This project follows the licenses of its dependencies. Please refer to the license files of each library.
