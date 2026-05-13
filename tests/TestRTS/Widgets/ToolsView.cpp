#include "ToolsView.h"

#include <imgui.h>

#include "../App/Tool.h"
#include "../App/ToolManager.h"
#include "../imgui/ImGuiPropertyGrid.h"

ToolsView::ToolsView(::ToolManager* toolManager)
    : ToolManager(toolManager)
{
}

void ToolsView::Render(Phoenix::SessionConstRef session, Phoenix::WorldConstRef world)
{
    auto tools = ToolManager->GetTools();

    // Toggle buttons — one per tool
    for (ITool* tool : tools)
    {
        const auto& descriptor = tool->GetTypeDescriptor();
        bool active = ToolManager->IsToolActive(tool);

        if (active)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
        }

        std::string buttonId = std::string(descriptor.GetDisplayName().c_str()) + "##toggle";
        if (ImGui::Button(buttonId.c_str()))
        {
            if (active)
            {
                ToolManager->DeactivateTool(tool);
            }
            else
            {
                ToolManager->ActivateTool(tool);

                // TODO (jfarris): handle auto activation/deactivation of other tools?
                // PlayerController is exclusive — disable everything else
                // if (tool == GPlayerController)
                // {
                //     GActiveTools.clear();
                //     GActiveTools.insert(GPlayerController);
                // }
                // else
                // {
                //     // Any non-PlayerController tool deactivates PlayerController
                //     GActiveTools.erase(GPlayerController);
                //
                //     // EntityTool and NavMeshTool require CameraTool
                //     if (tool == GEntityTool || tool == GNavMeshTool)
                //         GActiveTools.insert(GCameraTool);
                // }
            }
        }

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) && tool->GetDescription()[0] != '\0')
        {
            ImGui::SetTooltip("%s", tool->GetDescription());
        }

        if (active)
        {
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();
    }

    ImGui::NewLine();

    // Property foldouts for each active tool
    for (const auto& tool : tools)
    {
        if (!ToolManager->IsToolActive(tool))
        {
            continue;
        }

        const auto& descriptor = tool->GetTypeDescriptor();
        if (ImGui::CollapsingHeader(descriptor.GetDisplayName().c_str()))
        {
            if (tool->GetDescription()[0] != '\0')
            {
                ImGui::TextDisabled("%s", tool->GetDescription());
                ImGui::Spacing();
            }
            DrawPropertyGrid(tool, descriptor);
            tool->OnAppRenderUI();
        }
    }
}
