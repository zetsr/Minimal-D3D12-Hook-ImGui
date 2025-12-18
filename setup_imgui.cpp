#include "setup_imgui.h"

void SetupImGui(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags) {
    if (menu::isOpen) {
        ImGuiIO& io = ImGui::GetIO();
        static bool styleApplied = false;

        if (!styleApplied) {
            ImGuiStyle& style = ImGui::GetStyle();
            ImVec4* colors = style.Colors;

            style.WindowRounding = 12.0f;
            style.ChildRounding = 8.0f;
            style.FrameRounding = 8.0f;
            style.PopupRounding = 8.0f;
            style.ScrollbarRounding = 12.0f;
            style.GrabRounding = 8.0f;
            style.TabRounding = 8.0f;
            style.WindowBorderSize = 0.0f;
            style.FrameBorderSize = 0.0f;

            colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.13f, 0.94f);
            colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.16f, 0.80f);
            colors[ImGuiCol_TitleBg] = ImVec4(0.18f, 0.14f, 0.25f, 1.00f);
            colors[ImGuiCol_TitleBgActive] = ImVec4(0.24f, 0.18f, 0.35f, 1.00f);
            colors[ImGuiCol_Header] = ImVec4(0.28f, 0.20f, 0.40f, 0.60f);
            colors[ImGuiCol_HeaderHovered] = ImVec4(0.35f, 0.25f, 0.50f, 0.80f);
            colors[ImGuiCol_HeaderActive] = ImVec4(0.40f, 0.30f, 0.55f, 1.00f);
            colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.18f, 0.22f, 0.70f);
            colors[ImGuiCol_FrameBgHovered] = ImVec4(0.30f, 0.25f, 0.45f, 0.80f);
            colors[ImGuiCol_FrameBgActive] = ImVec4(0.35f, 0.28f, 0.50f, 1.00f);
            colors[ImGuiCol_CheckMark] = ImVec4(0.60f, 0.45f, 0.90f, 1.00f);
            colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 1.00f, 1.00f);
            colors[ImGuiCol_Separator] = ImVec4(0.35f, 0.30f, 0.50f, 0.50f);

            styleApplied = true;
        }

        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowBgAlpha(0.95f);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

        if (ImGui::Begin("Begeerte", nullptr, flags)) {
            if (ImGui::CollapsingHeader("Aimbot", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Indent(10.0f);
                static bool aimbot_enabled = false;
                static bool silent_aim = true;
                static bool fov_check = true;

                ImGui::Checkbox("Enable Aimbot", &aimbot_enabled);
                ImGui::Checkbox("Silent Aim", &silent_aim);
                ImGui::Checkbox("FOV Check", &fov_check);

                ImGui::Unindent(10.0f);
                ImGui::Spacing();
            }

            if (ImGui::CollapsingHeader("Visuals")) {
                ImGui::Indent(10.0f);
                static bool esp_box = true;
                static bool esp_name = true;
                static bool esp_health = false;
                static bool chams = false;

                ImGui::Checkbox("Box ESP", &esp_box);
                ImGui::Checkbox("Name ESP", &esp_name);
                ImGui::Checkbox("Health Bar", &esp_health);
                ImGui::Checkbox("Chams", &chams);

                ImGui::Unindent(10.0f);
                ImGui::Spacing();
            }

            if (ImGui::CollapsingHeader("Misc")) {
                ImGui::Indent(10.0f);
                static bool bunny_hop = false;
                static bool no_flash = true;
                static bool radar = true;

                ImGui::Checkbox("Bunny Hop", &bunny_hop);
                ImGui::Checkbox("No Flash", &no_flash);
                ImGui::Checkbox("Radar Hack", &radar);

                ImGui::TextColored(ImVec4(0.65f, 0.60f, 0.90f, 0.7f), "Press F1 to toggle menu");

                ImGui::Unindent(10.0f);
            }

            float windowHeight = ImGui::GetWindowHeight();
            float textHeight = ImGui::GetTextLineHeight();
            ImGui::SetCursorPosY(windowHeight - textHeight - ImGui::GetStyle().WindowPadding.y);

            ImGui::TextColored(ImVec4(0.60f, 0.55f, 0.85f, 0.8f), "github.com/zetsr");

            float versionWidth = ImGui::CalcTextSize("v1.0.0").x;
            ImGui::SameLine(ImGui::GetWindowWidth() - versionWidth - ImGui::GetStyle().WindowPadding.x);
            ImGui::TextColored(ImVec4(0.60f, 0.55f, 0.85f, 0.8f), "v1.0.0");
        }

        ImGui::End();
    }

    ImGui::GetForegroundDrawList()->AddText(ImVec2(5.0f, 5.0f), IM_COL32(255, 255, 0, 255), "Test Text");
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);
}