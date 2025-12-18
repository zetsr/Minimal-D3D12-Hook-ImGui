#include "setup_imgui.h"

void SetupImGui(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags) {
    if (menu::isOpen) {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowBgAlpha(1.0f);

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
                ImGui::Unindent(10.0f);
                ImGui::Spacing();
            }

            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.75f), "Press F1 to toggle menu");
        }

        ImGui::End();
    }

    ImGui::GetForegroundDrawList()->AddText(ImVec2(5.0f, 5.0f), IM_COL32(255, 255, 0, 255), "Test Text");
}