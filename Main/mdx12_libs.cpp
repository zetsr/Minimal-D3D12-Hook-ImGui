// 此文件专门用于承载第三方库（ImGui后端等）所需的静态链接
// 我们自己的 hook 代码通过运行时 GetProcAddress 调用 D3D12/DXGI，不依赖这些符号
// 但 imgui_impl_dx12 内部引用了 CreateDXGIFactory1 等符号，必须保留链接
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "user32.lib")