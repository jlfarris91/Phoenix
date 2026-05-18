#include "EditorFonts.h"

#include <imgui.h>

namespace Phoenix::UI::Shell
{
    EditorFonts LoadEditorFonts(const std::filesystem::path& dataDir, float size)
    {
        auto fontPath = [&](const char* rel) -> std::string
        {
            return (dataDir / rel).string();
        };

        ImGuiIO& io = ImGui::GetIO();

        EditorFonts fonts;
        fonts.Regular  = io.Fonts->AddFontFromFileTTF(fontPath("fonts/Noto_Sans/static/NotoSans-Regular.ttf").c_str(),  size);
        fonts.Medium   = io.Fonts->AddFontFromFileTTF(fontPath("fonts/Noto_Sans/static/NotoSans-Medium.ttf").c_str(),   size);
        fonts.SemiBold = io.Fonts->AddFontFromFileTTF(fontPath("fonts/Noto_Sans/static/NotoSans-SemiBold.ttf").c_str(), size);
        fonts.Bold     = io.Fonts->AddFontFromFileTTF(fontPath("fonts/Noto_Sans/static/NotoSans-Bold.ttf").c_str(),     size);

        // Fall back to the built-in font if any file failed to load.
        if (!fonts.Regular)  fonts.Regular  = io.Fonts->AddFontDefault();
        if (!fonts.Medium)   fonts.Medium   = fonts.Regular;
        if (!fonts.SemiBold) fonts.SemiBold = fonts.Regular;
        if (!fonts.Bold)     fonts.Bold     = fonts.Regular;

        return fonts;
    }
}
