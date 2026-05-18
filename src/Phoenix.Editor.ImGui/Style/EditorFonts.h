#pragma once

#include <imgui.h>
#include <filesystem>

namespace Phoenix::UI::Shell
{
    struct EditorFonts
    {
        ImFont* Regular  = nullptr;
        ImFont* Medium   = nullptr;
        ImFont* SemiBold = nullptr;
        ImFont* Bold     = nullptr;
    };

    // Loads fonts into the ImGui font atlas from dataDir (the exe's data/ folder).
    // Must be called after ImGui::CreateContext() and before the first frame.
    // Returns a populated EditorFonts struct — store it for use with PushFont/PopFont.
    EditorFonts LoadEditorFonts(const std::filesystem::path& dataDir, float size = 15.0f);

    // The globally loaded fonts — set by LoadEditorFonts, read by widgets.
    inline EditorFonts GFonts;
}
