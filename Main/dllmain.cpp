#include "mdx12_api.h"

// --- 1. 图标宏定义 (g_icomoon 映射) ---
#define ICON_AIMBOT    "A"
#define ICON_VISUALS   "B"
#define ICON_TEAM      "C"
#define ICON_MISC      "D"
#define ICON_CONFIG    "E"
#define ICON_GITHUB    "F"
#define ICON_LOCK      "G"
#define ICON_UNLOCK    "H"
#define ICON_REFRESH   "I"

float g_MenuAlpha = 0.0f;

void MyImGuiDraw(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{

    static bool g_Locked = false;
    static int selectedMainTab = 0;
    constexpr ImGuiColorEditFlags MinimalColorFlags =
        ImGuiColorEditFlags_AlphaBar |
        ImGuiColorEditFlags_DisplayHex |
        ImGuiColorEditFlags_NoSidePreview |
        ImGuiColorEditFlags_NoSmallPreview |
        ImGuiColorEditFlags_NoOptions |
        ImGuiColorEditFlags_NoTooltip |
        ImGuiColorEditFlags_NoLabel;

    // --- 1. 尺寸初始化 ---
    const float FADE_SPEED = 5.0f;
    static bool firstRun = true;
	static ImVec2 firstSize;
    ImGuiIO& io = ImGui::GetIO();
    float deltaTime = io.DeltaTime;

    if (firstRun) {
        float baseHeight = io.DisplaySize.y * 0.33f;
        firstSize = ImVec2(baseHeight * (16.0f / 10.0f), baseHeight);
        ImGui::SetNextWindowSize(firstSize, ImGuiCond_FirstUseEver);
        firstRun = false;
    }
    ImGui::SetNextWindowSizeConstraints(firstSize, io.DisplaySize);

    // --- 2. 交互配置 ---
    ImGui::GetIO().ConfigWindowsResizeFromEdges = false;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar;
    if (g_Locked) window_flags |= ImGuiWindowFlags_NoResize;

    // --- 3. 样式推入 ---
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, g_MenuAlpha);

    if (g_MDX12::g_MenuState::g_isOpen) {
        g_MenuAlpha += io.DeltaTime * FADE_SPEED;
        if (g_MenuAlpha > 1.0f) g_MenuAlpha = 1.0f;
    }
    else {
        g_MenuAlpha -= io.DeltaTime * FADE_SPEED;
        if (g_MenuAlpha < 0.0f) g_MenuAlpha = 0.0f;
    }

    if (g_MenuAlpha <= 0.001f) return;

    if (ImGui::Begin("Pro Adaptive Menu", &g_MDX12::g_MenuState::g_isOpen, window_flags))
    {
        ImVec2 winSize = ImGui::GetWindowSize();
        ImGuiStyle& style = ImGui::GetStyle();
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        style.Colors[ImGuiCol_WindowBg].w = g_MenuAlpha > 0.9 ? 0.9 : g_MenuAlpha;

        // [侧边栏参数计算]
        ImGui::PushFont(g_MDX12::g_icomoon);
        const float sideBarW = ImGui::CalcTextSize(ICON_AIMBOT).x + (style.FramePadding.x * 8.0f);
        const float tabH = 60.0f;
        ImGui::PopFont();

        // [渲染左侧边栏背景]
        ImVec2 p = ImGui::GetCursorScreenPos();
        drawList->AddRectFilled(p, ImVec2(p.x + sideBarW, p.y + winSize.y), ImGui::GetColorU32(ImGuiCol_ChildBg));

        // [渲染左侧边栏按钮]
        ImGui::BeginGroup();
        {
            const char* tabIcons[] = { ICON_AIMBOT, ICON_VISUALS, ICON_TEAM, ICON_MISC, ICON_CONFIG };

            for (int i = 0; i < std::ssize(tabIcons); i++) {
                ImVec2 cursorPos = ImGui::GetCursorScreenPos();
                bool isSelected = (selectedMainTab == i);

                // 1. 正常的隐形按钮：保持原本满宽（sideBarW）和满高（tabH），绝不破坏垂直排版
                ImGui::PushID(i);
                if (ImGui::InvisibleButton("##TabBtn", ImVec2(sideBarW, tabH))) {
                    selectedMainTab = i;
                }
                bool isHovered = ImGui::IsItemHovered();
                ImGui::PopID();

                // 2. 靠右对齐的背景与图标参数计算
                ImGui::PushFont(g_MDX12::g_icomoon);
                ImVec2 iconSize = ImGui::CalcTextSize(tabIcons[i]);
                ImGui::PopFont();

                // 自动计算水平 Padding 留白
                float paddingX = style.FramePadding.x * 2.0f;
                // 动态高亮块的宽度 = 图标宽度 + 左右留白
                float highlightW = iconSize.x + (paddingX * 2.0f);
                // 背景块的起始 X 坐标（让其靠右）
                float btnStartX = cursorPos.x + sideBarW - highlightW;
                ImU32 textColor = ImGui::GetColorU32(isSelected ? ImGuiCol_Text : (isHovered ? ImGuiCol_Text : ImGuiCol_TextDisabled));

                // 3. 绘制高亮背景（垂直坐标依旧采用原汁原味的 cursorPos.y，确保垂直完全不变）
                if (isSelected) {
                    // 选中状态：右对齐的矩形背景
                    // drawList->AddRectFilled(ImVec2(btnStartX, cursorPos.y), ImVec2(btnStartX + highlightW, cursorPos.y + tabH), ImGui::GetColorU32(ImGuiCol_HeaderActive));
                    // 激活指示线：紧贴在右对齐矩形的最左侧
                    // drawList->AddRectFilled(ImVec2(btnStartX, cursorPos.y), ImVec2(btnStartX + 3.0f, cursorPos.y + tabH), ImGui::GetColorU32(ImGuiCol_CheckMark));
                }
                else if (isHovered) {
                    // 悬浮状态：右对齐的矩形背景
                    // drawList->AddRectFilled(ImVec2(btnStartX, cursorPos.y), ImVec2(btnStartX + highlightW, cursorPos.y + tabH), ImGui::GetColorU32(ImGuiCol_HeaderHovered));
                }

                // 4. 绘制图标（垂直对齐算法恢复成你最初完全正确的逻辑）
                ImGui::PushFont(g_MDX12::g_icomoon);
                ImVec2 iconPos = ImVec2(
                    btnStartX + paddingX,                  // 水平靠右并留出 Padding
                    cursorPos.y + (tabH - iconSize.y) * 0.5f // 垂直居中（与你最初的代码完全一致）
                );
                drawList->AddText(iconPos, textColor, tabIcons[i]);
                ImGui::PopFont();
            }
        }
        ImGui::EndGroup();

        ImGui::SameLine();

        // [内容主体区]
        ImGui::BeginGroup();
        {
            const float contentPadding = 20.0f;
            const float topAnchorY = 15.0f;

            ImGui::PushFont(g_MDX12::g_icomoon_small);
            float smIconH = ImGui::GetTextLineHeight();
            float iconsWidth = (ImGui::CalcTextSize(ICON_GITHUB).x * 2) + 25.0f;
            ImGui::PopFont();
            float textH = ImGui::GetTextLineHeight();

            ImGui::Dummy(ImVec2(0, topAnchorY));

            ImGui::Indent(contentPadding);
            ImGui::BeginGroup();
            {
                float titleYOffset = (smIconH - textH) * 0.5f;
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + titleYOffset);

                const char* tabNames[] = { "AIMBOT MODULE", "VISUALS MODULE", "PLAYERS", "SETTINGS", "CONFIGS" };
                ImGui::PushFont(g_MDX12::g_Alibaba_PuHuiTi_Bold);
                ImGui::TextColored(style.Colors[ImGuiCol_PlotLines], tabNames[selectedMainTab]);
                ImGui::PopFont();

                ImGui::SameLine(ImGui::GetContentRegionAvail().x - iconsWidth);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() - titleYOffset);

                ImGui::PushFont(g_MDX12::g_icomoon_small);
                if (ImGui::Selectable(g_Locked ? ICON_LOCK : ICON_UNLOCK, false, 0, ImVec2(smIconH, smIconH))) g_Locked = !g_Locked;
                ImGui::SameLine(0, 15.0f);
                if (ImGui::Selectable(ICON_GITHUB, false, 0, ImVec2(smIconH, smIconH))) ShellExecuteA(NULL, "open", "https://github.com", NULL, NULL, SW_SHOWNORMAL);
                ImGui::PopFont();
            }
            ImGui::EndGroup();

            ImGui::Dummy(ImVec2(0, 20.0f));

            const float gridSpacing = 15.0f;
            const float availWidth = ImGui::GetContentRegionAvail().x - contentPadding;
            const float itemW = (availWidth - gridSpacing) / 2.0f;

            // 动态计算剩余高度：当前可用高度减去底部间距
            const float dynamicChildHeight = ImGui::GetContentRegionAvail().y - contentPadding;

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 12.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);

            ImVec4 childBg = style.Colors[ImGuiCol_FrameBg];
            childBg.w = style.Colors[ImGuiCol_WindowBg].w;
            ImGui::PushStyleColor(ImGuiCol_ChildBg, childBg);

            ImGui::PushStyleColor(ImGuiCol_Border, style.Colors[ImGuiCol_Border]);

            if (selectedMainTab == 0) // AIMBOT
            {
                ImGui::BeginChild("##AimbotGen", ImVec2(itemW, dynamicChildHeight), true);
                {
                    ImGui::Indent(10); ImGui::Spacing();
                    ImGui::TextDisabled("Automation"); ImGui::Separator();
                    static bool aim_enable = false, aim_silent = false, aim_team = false;
                    ImGui::Checkbox("Enable Aimbot", &aim_enable);
                    ImGui::Checkbox("Silent Aim", &aim_silent);
                    ImGui::Checkbox("Target Teammates", &aim_team);
                }
                ImGui::EndChild();

                ImGui::SameLine(0, gridSpacing);

                ImGui::BeginChild("##AimbotAcc", ImVec2(itemW, dynamicChildHeight), true);
                {
                    ImGui::Indent(10); ImGui::Spacing();
                    ImGui::TextDisabled("Accuracy"); ImGui::Separator();
                    static float aim_fov = 124.2f, aim_smooth = 14.5f;
                    static int aim_bone = 3; // Pelvis

                    ImGui::Text("Field of View");
                    ImGui::SetNextItemWidth(-10);
                    ImGui::SliderFloat("##FOV", &aim_fov, 1.0f, 180.0f, "%.0f");

                    ImGui::Text("Smoothing");
                    ImGui::SetNextItemWidth(-10);
                    ImGui::SliderFloat("##Smooth", &aim_smooth, 1.0f, 30.0f, "%.1f");

                    ImGui::Text("Target Bone");
                    const char* bones[] = { "Head", "Neck", "Chest", "Pelvis" };
                    ImGui::SetNextItemWidth(-10);
                    ImGui::Combo("##Bone", &aim_bone, bones, IM_ARRAYSIZE(bones));
                }
                ImGui::EndChild();
            }
            else if (selectedMainTab == 1) // VISUALS
            {
                ImGui::BeginChild("##EspGen", ImVec2(itemW, dynamicChildHeight), true);
                {
                    ImGui::Indent(10); ImGui::Spacing();
                    ImGui::TextDisabled("Overlay"); ImGui::Separator();
                    static bool esp_box = false, esp_name = false, esp_health = false, esp_line = false;
                    static float line_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
                    static float box_color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };

                    ImGui::Checkbox("Bounding Box", &esp_box);
                    ImGui::SameLine(ImGui::GetWindowWidth() - 40);

                    if (ImGui::ColorButton("##BoxColBtn", ImVec4(box_color[0], box_color[1], box_color[2], box_color[3]), ImGuiColorEditFlags_NoTooltip)) {
                        ImGui::OpenPopup("BoxColPopup");
                    }

                    if (ImGui::BeginPopup("BoxColPopup")) {
                        ImGui::ColorPicker4("##BoxPicker", box_color, MinimalColorFlags);
                        ImGui::EndPopup();
                    }

                    ImGui::Checkbox("Player Names", &esp_name);
                    ImGui::Checkbox("Health Bar", &esp_health);

                    ImGui::Checkbox("Snap Lines", &esp_line);
                    ImGui::SameLine(ImGui::GetWindowWidth() - 40);

                    if (ImGui::ColorButton("##LineColBtn", ImVec4(line_color[0], line_color[1], line_color[2], line_color[3]), ImGuiColorEditFlags_NoTooltip)) {
                        ImGui::OpenPopup("LineColPopup");
                    }

                    if (ImGui::BeginPopup("LineColPopup")) {
                        ImGui::ColorPicker4("##LinePicker", box_color, MinimalColorFlags);
                        ImGui::EndPopup();
                    }

                }
                ImGui::EndChild();

                ImGui::SameLine(0, gridSpacing);

                ImGui::BeginChild("##ChamsGen", ImVec2(itemW, dynamicChildHeight), true);
                {
                    ImGui::Indent(10); ImGui::Spacing();
                    ImGui::TextDisabled("Models"); ImGui::Separator();
                    static bool chams_en = false, chams_xray = false;
                    static float chams_col_vis[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
                    static float chams_col_hid[4] = { 1.0f, 1.0f, 0.0f, 1.0f };

                    ImGui::Checkbox("Enable Chams", &chams_en);
                    ImGui::Checkbox("X-Ray (Wall)", &chams_xray);

                    ImGui::Spacing();
                    ImGui::TextDisabled("Colors");

                    if (ImGui::ColorButton("##ChamsVisColBtn", ImVec4(chams_col_vis[0], chams_col_vis[1], chams_col_vis[2], chams_col_vis[3]), ImGuiColorEditFlags_NoTooltip)) {
                        ImGui::OpenPopup("ChamsVisColPopup");
                    }

                    if (ImGui::BeginPopup("ChamsVisColPopup")) {
                        ImGui::ColorPicker4("##ChamsVisPicker", chams_col_vis, MinimalColorFlags);
                        ImGui::EndPopup();
                    }
                    ImGui::SameLine(); ImGui::Text("Visible Color");

                    if (ImGui::ColorButton("##ChamsHidColBtn", ImVec4(chams_col_hid[0], chams_col_hid[1], chams_col_hid[2], chams_col_hid[3]), ImGuiColorEditFlags_NoTooltip)) {
                        ImGui::OpenPopup("ChamsHidColPopup");
                    }

                    if (ImGui::BeginPopup("ChamsHidColPopup")) {
                        ImGui::ColorPicker4("##ChamsHidPicker", chams_col_hid, MinimalColorFlags);
                        ImGui::EndPopup();
                    }
                    ImGui::SameLine(); ImGui::Text("Hidden Color");
                }
                ImGui::EndChild();
            }
            else if (selectedMainTab == 2) // PLAYERS
            {
                ImGui::BeginChild("##PlayerList", ImVec2(availWidth, dynamicChildHeight), true);
                {
                    ImGui::Indent(10); ImGui::Spacing();
                    ImGui::TextDisabled("Active Players in Session"); ImGui::Separator();

                    static int selectedPlayer = -1;
                    const char* players[] = { "Player_001 (Level 50)", "Admin_Alpha (Level 999)", "User_123 (Level 12)", "Unknown_Entity" };

                    if (ImGui::BeginListBox("##plist", ImVec2(-10, -80))) {
                        for (int n = 0; n < IM_ARRAYSIZE(players); n++) {
                            const bool is_selected = (selectedPlayer == n);
                            if (ImGui::Selectable(players[n], is_selected)) selectedPlayer = n;
                        }
                        ImGui::EndListBox();
                    }

                    if (selectedPlayer != -1) {
                        if (ImGui::Button("Teleport", ImVec2(120, 30))) { /* Teleport Logic */ }
                        ImGui::SameLine();
                        if (ImGui::Button("Spectate", ImVec2(120, 30))) { /* Spectate Logic */ }
                        ImGui::SameLine();
                        if (ImGui::Button("Force Crash", ImVec2(120, 30))) { /* Exploit Logic */ }
                    }
                }
                ImGui::EndChild();
            }
            else if (selectedMainTab == 3) // SETTINGS
            {
                ImGui::BeginChild("##MenuSet", ImVec2(itemW, dynamicChildHeight), true);
                {
                    ImGui::Indent(10); ImGui::Spacing();
                    ImGui::TextDisabled("Interface Settings"); ImGui::Separator();

                    ImGui::Text("Menu Transparency");
                    ImGui::SetNextItemWidth(-10);

                    static bool show_fps = true;
                    ImGui::Checkbox("Show Overlay FPS", &show_fps);
                }
                ImGui::EndChild();

                ImGui::SameLine(0, gridSpacing);

                ImGui::BeginChild("##MiscSet", ImVec2(itemW, dynamicChildHeight), true);
                {
                    ImGui::Indent(10); ImGui::Spacing();
                    ImGui::TextDisabled("Exploits"); ImGui::Separator();
                    static bool bhop = false, noclip = false;
                    static float speed_mult = 1.0f;

                    ImGui::Checkbox("Auto BunnyHop", &bhop);
                    ImGui::Checkbox("No Clip (N)", &noclip);

                    ImGui::Text("Speed Multiplier");
                    ImGui::SetNextItemWidth(-10);
                    ImGui::SliderFloat("##Speed", &speed_mult, 1.0f, 10.0f, "x%.1f");
                }
                ImGui::EndChild();
            }
            else if (selectedMainTab == 4) // CONFIGS
            {
                ImGui::BeginChild("##ConfigList", ImVec2(availWidth, dynamicChildHeight), true);
                {
                    ImGui::Indent(10); ImGui::Spacing();
                    ImGui::TextDisabled("Configuration Profiles"); ImGui::Separator();

                    static char cfgName[64] = "Legit_v1.2";
                    ImGui::Text("Profile Name");
                    ImGui::SetNextItemWidth(300);
                    ImGui::InputText("##Name", cfgName, 64);

                    ImGui::Spacing();
                    if (ImGui::Button("Save Profile", ImVec2(140, 35))) { /* Save logic */ }
                    ImGui::SameLine();
                    if (ImGui::Button("Load Selected", ImVec2(140, 35))) { /* Load logic */ }
                    ImGui::SameLine();
                    if (ImGui::Button("Delete", ImVec2(140, 35))) { /* Delete logic */ }

                    ImGui::Spacing(); ImGui::Separator();
                    ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "Status: Configuration Loaded Successfully.");
                }
                ImGui::EndChild();
            }

            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar(2);
            ImGui::Unindent(contentPadding);
        }
        ImGui::EndGroup();
    }
    ImGui::End();
    ImGui::PopStyleVar(6);
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