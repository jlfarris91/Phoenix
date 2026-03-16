#include "ImGuiUtils.h"

#include "imgui.h"

void WrapPanel(const std::vector<std::string>& labels, size_t& selected)
{
    float current_x = 0.0f;
    ImGuiStyle& style = ImGui::GetStyle();

    for (size_t i = 0; i < labels.size(); ++i)
    {
        // Calculate the size of the button before drawing it
        ImVec2 button_size = ImGui::CalcTextSize(labels[i].c_str());
        // Add FramePadding and ItemSpacing for accurate width calculation
        float button_width = button_size.x + style.FramePadding.x * 2.0f + style.ItemSpacing.x;

        // Check if the button fits within the remaining width
        if (current_x + button_width >= ImGui::GetContentRegionAvail().x)
        {
            // If not, reset position to the start of a new line
            ImGui::NewLine();
            current_x = 0.0f;
        }

        // Draw the button
        if (ImGui::RadioButton(labels[i].c_str(), selected == i))
        {
            selected = i;
        }

        // Use SameLine to place the next item horizontally
        ImGui::SameLine();
        current_x += button_width;
    }
    // Ensure the cursor ends on a new line after the loop
    ImGui::NewLine();
}
