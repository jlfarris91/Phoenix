#include "InspectorView.h"

#include <imgui.h>

#include "../imgui/ImGuiPropertyGrid.h"

#include "PhoenixSim/ECS/EntityId.h"
#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixRTS/Selection/FeatureSelection.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

void InspectorView::Render(SessionConstRef session, WorldConstRef world)
{
    EntityId playerSelection = FeatureSelection::GetPlayerSelection(world, 0);
    EntityId selectedEntityId = FeatureECS::GetFirstEntityInGroup(world, playerSelection);

    const Entity* selectedEntity = FeatureECS::GetEntityPtr(world, selectedEntityId);
    if (!selectedEntity)
    {
        ImGui::Text("Select an entity");
        return;
    }

    if (ImGui::BeginTable("Info", 2, ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableNextColumn();
        ImGui::Text("Entity Id:");
        ImGui::TableNextColumn();
        ImGui::Text("%u", (uint32)selectedEntity->Id);

        ImGui::TableNextColumn();
        ImGui::Text("Kind:");
        ImGui::TableNextColumn();
        ImGui::Text("%s", FName::GetNameEntry(selectedEntity->Kind));

        ImGui::EndTable();
    }

    if (ImGui::TreeNode("Components:"))
    {
        FeatureECS::ForEachComponent(world, selectedEntityId, [&](const ComponentDefinition& compDef, const void* comp)
        {
            if (compDef.TypeDescriptor && ImGui::TreeNode(compDef.TypeDescriptor->GetDisplayName().c_str()))
            {
                DrawPropertyGrid(comp, *compDef.TypeDescriptor);
                ImGui::TreePop();
            }
        });

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Group:"))
    {
        if (ImGui::BeginTable("Entities", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("Index");
            ImGui::TableSetupColumn("Id");
            ImGui::TableSetupColumn("Kind");
            ImGui::TableHeadersRow();

            const auto& entities = *FeatureECS::GetEntities(world);
            FeatureECS::ForEachEntityInGroup(world, selectedEntityId, [&](EntityId childId)
            {
                if (const Entity* childEntity = FeatureECS::GetEntityPtr(world, childId))
                {
                    ImGui::TableNextColumn();
                    ImGui::Text("%u:", entities.GetEntityIndex(childId));
                    ImGui::TableNextColumn();
                    ImGui::Text("%u", (uint32)childEntity->Id);
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", FName::GetNameEntry(childEntity->Kind));
                }
            });

            ImGui::EndTable();
        }

        ImGui::TreePop();
    }
}
