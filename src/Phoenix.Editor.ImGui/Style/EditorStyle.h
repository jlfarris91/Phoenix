#pragma once

namespace Phoenix::UI::Shell
{
    // Applies the Phoenix custom ImGui style.
    // Must be called after ImGui::CreateContext() and before the first frame.
    void ApplyEditorStyle();
}
