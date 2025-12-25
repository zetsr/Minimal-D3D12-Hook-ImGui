#include "mdx12_api.h"

// --- 第一部分：将样式推向极致的初始化函数 ---
void SetupExtremeStyle() {
    auto& style = ImGui::GetStyle();
    auto& colors = style.Colors;

    // [布局设置]
    style.WindowRounding = 12.0f;           // 窗口大圆角
    style.FrameRounding = 6.0f;            // 控件圆角
    style.ItemSpacing = ImVec2(12, 12);     // 增大控件间距，呼吸感
    style.ScrollbarSize = 10.0f;           // 细长的滚动条
    style.ScrollbarRounding = 9.0f;
    style.WindowPadding = ImVec2(20, 20);
    style.FramePadding = ImVec2(10, 8);
    style.WindowBorderSize = 0.0f;          // 去掉生硬的边框

    // [颜色方案] - 采用深邃的紫色/青色渐变感
    colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.10f, 0.94f); // 极深背景
    colors[ImGuiCol_Header] = ImVec4(0.25f, 0.25f, 0.35f, 0.45f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.35f, 0.35f, 0.50f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.45f, 0.45f, 0.65f, 1.00f);

    colors[ImGuiCol_Button] = ImVec4(0.15f, 0.15f, 0.20f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.25f, 0.35f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.60f, 0.40f, 0.90f, 1.00f); // 点击时的霓虹紫

    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);

    colors[ImGuiCol_CheckMark] = ImVec4(0.70f, 0.50f, 1.00f, 1.00f); // 勾选框紫色
    colors[ImGuiCol_SliderGrab] = ImVec4(0.50f, 0.40f, 0.90f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.70f, 0.60f, 1.00f, 1.00f);

    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
}
void MyImGuiDraw(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{
    // 静态标记，确保样式只加载一次
    static bool isInitialized = false;
    if (!isInitialized) {
        SetupExtremeStyle();
        isInitialized = true;
    }

    if (g_MDX12::g_MenuState::g_isOpen) {
        // 设置窗口出现的动画感（固定位置和大小）
        ImGui::SetNextWindowPos(ImVec2(150, 150), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

        // 使用特殊 Flag 去掉标题栏（NoTitleBar），我们要自己画一个高端的标题栏
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

        if (ImGui::Begin("CyberMenu", nullptr, window_flags)) {
            // 1. 自定义标题栏绘制 (用 DrawList 画一条装饰线)
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 p = ImGui::GetCursorScreenPos();
            draw_list->AddRectFilled(ImVec2(p.x - 20, p.y - 20), ImVec2(p.x + 480, p.y + 10), ImColor(30, 30, 45, 255));
            draw_list->AddText(ImVec2(p.x + 5, p.y - 15), ImColor(200, 200, 255), "PREMIUM DASHBOARD v1.0");

            ImGui::Dummy(ImVec2(0, 25)); // 占位
            ImGui::Separator();
            ImGui::Spacing();

            // 2. 左侧侧边栏模拟
            ImGui::BeginGroup();
            {
                if (ImGui::Button(" DASHBOARD ", ImVec2(120, 40))) { /* 逻辑 */ }
                if (ImGui::Button(" SETTINGS  ", ImVec2(120, 40))) { /* 逻辑 */ }
                if (ImGui::Button(" NETWORK   ", ImVec2(120, 40))) { /* 逻辑 */ }

                // 画一个炫酷的渐变边框装饰
                ImVec2 rect_p = ImGui::GetCursorScreenPos();
                draw_list->AddRectFilledMultiColor(ImVec2(rect_p.x, rect_p.y + 100), ImVec2(rect_p.x + 120, rect_p.y + 102),
                    ImColor(150, 50, 250, 255), ImColor(50, 150, 250, 255),
                    ImColor(50, 150, 250, 255), ImColor(150, 50, 250, 255));
            }
            ImGui::EndGroup();

            ImGui::SameLine();
            ImGui::Spacing();
            ImGui::SameLine();

            // 3. 右侧主内容区
            ImGui::BeginChild("MainContent", ImVec2(0, 0), true, ImGuiWindowFlags_NoBackground);
            {
                ImGui::TextColored(ImVec4(0.6f, 0.5f, 1.0f, 1.0f), "SYSTEM STATUS: SECURED");
                ImGui::Spacing();

                static float health = 0.85f;
                ImGui::Text("Engine Stability");
                ImGui::ProgressBar(health, ImVec2(-1, 20), "");

                static bool enable_esp = true;
                ImGui::Checkbox("Enable Neural Link", &enable_esp);

                static int mode = 0;
                ImGui::RadioButton("Aggressive", &mode, 0); ImGui::SameLine();
                ImGui::RadioButton("Stealth", &mode, 1);

                static float color[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
                ImGui::ColorEdit4("Aura Color", color, ImGuiColorEditFlags_NoInputs);
            }
            ImGui::EndChild();
        }
        ImGui::End(); // 永远记得 End 在这里！
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