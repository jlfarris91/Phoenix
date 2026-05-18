#include "EditorStyle.h"

#include <imgui.h>

namespace Phoenix::UI::Shell
{
    void ApplyEditorStyle()
    {
        ImGuiStyle& style = ImGui::GetStyle();

        // --- Shape ---
        style.WindowRounding     = 4.0f;
        style.ChildRounding      = 4.0f;
        style.FrameRounding      = 3.0f;
        style.PopupRounding      = 4.0f;
        style.ScrollbarRounding  = 3.0f;
        style.GrabRounding       = 3.0f;
        style.TabRounding        = 4.0f;

        // --- Spacing ---
        style.WindowPadding      = ImVec2(8.0f,  8.0f);
        style.FramePadding       = ImVec2(5.0f,  3.0f);
        style.ItemSpacing        = ImVec2(6.0f,  4.0f);
        style.ItemInnerSpacing   = ImVec2(4.0f,  4.0f);
        style.IndentSpacing      = 16.0f;
        style.ScrollbarSize      = 12.0f;
        style.GrabMinSize        = 8.0f;
        style.TabBarBorderSize   = 1.0f;

        // --- Borders ---
        style.WindowBorderSize   = 1.0f;
        style.ChildBorderSize    = 1.0f;
        style.PopupBorderSize    = 1.0f;
        style.FrameBorderSize    = 0.0f;

        // --- Palette ---
        // bg0 = darkest (app background, title bars)
        // bg1 = window background
        // bg2 = child / popup background
        // bg3 = frame / input field background
        // bg4 = hovered state
        // bg5 = active/pressed state
        const ImVec4 bg0        = { 0.11f, 0.11f, 0.12f, 1.00f };
        const ImVec4 bg1        = { 0.14f, 0.14f, 0.16f, 1.00f };
        const ImVec4 bg2        = { 0.18f, 0.18f, 0.20f, 1.00f };
        const ImVec4 bg3        = { 0.22f, 0.22f, 0.25f, 1.00f };
        const ImVec4 bg4        = { 0.28f, 0.28f, 0.32f, 1.00f };
        const ImVec4 bg5        = { 0.34f, 0.34f, 0.40f, 1.00f };
        const ImVec4 accent     = { 0.28f, 0.56f, 0.90f, 1.00f };
        const ImVec4 accentHov  = { 0.36f, 0.64f, 0.96f, 1.00f };
        const ImVec4 accentAct  = { 0.22f, 0.48f, 0.82f, 1.00f };
        const ImVec4 textPrim   = { 0.90f, 0.90f, 0.92f, 1.00f };
        const ImVec4 textDisab  = { 0.38f, 0.38f, 0.42f, 1.00f };
        const ImVec4 border     = { 0.22f, 0.22f, 0.26f, 1.00f };

        ImVec4* c = style.Colors;
        c[ImGuiCol_Text]                    = textPrim;
        c[ImGuiCol_TextDisabled]            = textDisab;
        c[ImGuiCol_WindowBg]                = bg1;
        c[ImGuiCol_ChildBg]                 = bg0;
        c[ImGuiCol_PopupBg]                 = bg2;
        c[ImGuiCol_Border]                  = border;
        c[ImGuiCol_BorderShadow]            = { 0.00f, 0.00f, 0.00f, 0.00f };
        c[ImGuiCol_FrameBg]                 = bg3;
        c[ImGuiCol_FrameBgHovered]          = bg4;
        c[ImGuiCol_FrameBgActive]           = bg5;
        c[ImGuiCol_TitleBg]                 = bg0;
        c[ImGuiCol_TitleBgActive]           = { 0.14f, 0.28f, 0.46f, 1.00f };
        c[ImGuiCol_TitleBgCollapsed]        = bg0;
        c[ImGuiCol_MenuBarBg]               = bg0;
        c[ImGuiCol_ScrollbarBg]             = bg0;
        c[ImGuiCol_ScrollbarGrab]           = bg4;
        c[ImGuiCol_ScrollbarGrabHovered]    = bg5;
        c[ImGuiCol_ScrollbarGrabActive]     = accent;
        c[ImGuiCol_CheckMark]               = accent;
        c[ImGuiCol_SliderGrab]              = accent;
        c[ImGuiCol_SliderGrabActive]        = accentAct;
        c[ImGuiCol_Button]                  = bg3;
        c[ImGuiCol_ButtonHovered]           = bg4;
        c[ImGuiCol_ButtonActive]            = bg5;
        c[ImGuiCol_Header]                  = { 0.28f, 0.56f, 0.90f, 0.20f };
        c[ImGuiCol_HeaderHovered]           = { 0.28f, 0.56f, 0.90f, 0.35f };
        c[ImGuiCol_HeaderActive]            = { 0.28f, 0.56f, 0.90f, 0.50f };
        c[ImGuiCol_Separator]               = border;
        c[ImGuiCol_SeparatorHovered]        = accentHov;
        c[ImGuiCol_SeparatorActive]         = accent;
        c[ImGuiCol_ResizeGrip]              = { 0.28f, 0.56f, 0.90f, 0.20f };
        c[ImGuiCol_ResizeGripHovered]       = { 0.28f, 0.56f, 0.90f, 0.60f };
        c[ImGuiCol_ResizeGripActive]        = { 0.28f, 0.56f, 0.90f, 0.90f };
        c[ImGuiCol_Tab]                     = bg2;
        c[ImGuiCol_TabHovered]              = bg4;
        c[ImGuiCol_TabSelected]             = bg3;
        c[ImGuiCol_TabSelectedOverline]     = accent;
        c[ImGuiCol_TabDimmed]               = bg1;
        c[ImGuiCol_TabDimmedSelected]       = bg2;
        c[ImGuiCol_TabDimmedSelectedOverline] = { 0.28f, 0.56f, 0.90f, 0.40f };
        c[ImGuiCol_DockingPreview]          = { 0.28f, 0.56f, 0.90f, 0.50f };
        c[ImGuiCol_DockingEmptyBg]          = bg0;
        c[ImGuiCol_PlotLines]               = accent;
        c[ImGuiCol_PlotLinesHovered]        = accentHov;
        c[ImGuiCol_PlotHistogram]           = accent;
        c[ImGuiCol_PlotHistogramHovered]    = accentHov;
        c[ImGuiCol_TableHeaderBg]           = bg0;
        c[ImGuiCol_TableBorderStrong]       = border;
        c[ImGuiCol_TableBorderLight]        = { 0.18f, 0.18f, 0.20f, 1.00f };
        c[ImGuiCol_TableRowBg]              = { 0.00f, 0.00f, 0.00f, 0.00f };
        c[ImGuiCol_TableRowBgAlt]           = { 1.00f, 1.00f, 1.00f, 0.03f };
        c[ImGuiCol_TextSelectedBg]          = { 0.28f, 0.56f, 0.90f, 0.35f };
        c[ImGuiCol_DragDropTarget]          = { 1.00f, 0.84f, 0.00f, 0.90f };
        c[ImGuiCol_NavHighlight]            = accent;
        c[ImGuiCol_NavWindowingHighlight]   = { 1.00f, 1.00f, 1.00f, 0.70f };
        c[ImGuiCol_NavWindowingDimBg]       = { 0.80f, 0.80f, 0.80f, 0.20f };
        c[ImGuiCol_ModalWindowDimBg]        = { 0.08f, 0.08f, 0.10f, 0.60f };
    }
}
