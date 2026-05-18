#include "ImGuiControls.h"

#include <imgui.h>
#include <string>

void ImGui::SectionHeader(const char* fmt, ...)
{
    // ImGui::form
    //
    // ImDrawList* dl       = ImGui::GetWindowDrawList();
    // ImVec2      pos      = ImGui::GetCursorScreenPos();
    // float       width    = ImGui::GetContentRegionAvail().x;
    // ImVec2      textSize = ImGui::CalcTextSize(label);
    //
    // float height  = textSize.y + 6.0f;
    // float lineY   = pos.y + textSize.y * 0.5f;
    // float lineX   = pos.x + textSize.x + 8.0f;
    //
    // // Reserve layout space FIRST — before any drawing
    // ImGui::Dummy(ImVec2(width, height));
    //
    // // Draw text
    // dl->AddText(pos, ImGui::GetColorU32(ImGuiCol_TextDisabled), label);
    //
    // // Horizontal rule to the right of the label
    // dl->AddLine(
    //     ImVec2(lineX,           lineY),
    //     ImVec2(pos.x + width,   lineY),
    //     ImGui::GetColorU32(ImGuiCol_Separator)
    // );
}
