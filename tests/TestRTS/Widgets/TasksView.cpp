#include "TasksView.h"

#include <imgui.h>

#include "PhoenixSim/Tasks/FeatureTask.h"

using namespace Phoenix;
using namespace Phoenix::Tasks;

void TasksView::Render(SessionConstRef session, WorldConstRef world)
{
    const FeatureTaskDynamicBlock& taskBlock = world.GetBlockRef<FeatureTaskDynamicBlock>();

    if (ImGui::BeginTable("Stats", 2, ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableNextColumn();
        ImGui::Text("Num Tasks:");
        ImGui::TableNextColumn();
        ImGui::Text("%u / %u / %u", taskBlock.Tasks.GetNumValid(), taskBlock.Tasks.GetNum(), taskBlock.Tasks.GetCapacity());

        ImGui::TableNextColumn();
        ImGui::Text("Num Blocks:");
        ImGui::TableNextColumn();
        ImGui::Text("%u / %u / %u", taskBlock.Tasks.GetNumOccupiedBlocks(), taskBlock.Tasks.GetNumBlocks(), taskBlock.Tasks.GetBlockCapacity());

        ImGui::EndTable();
    }

    if (ImGui::BeginTable("Tasks", 5, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("Tick");
        ImGui::TableSetupColumn("Context");
        ImGui::TableSetupColumn("Id");
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Handle");
        ImGui::TableHeadersRow();

        for (const TaskEntry& taskEntry : taskBlock.Tasks)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%+.2f", (double)(taskEntry.GetTickTime() - world.GetSimTime()));
            ImGui::TableNextColumn();
            ImGui::Text("%u", taskEntry.GetContext());
            ImGui::TableNextColumn();
            ImGui::Text("%s", FName::GetNameEntry(taskEntry.GetId()));
            ImGui::TableNextColumn();
            ImGui::Text("%s", FName::GetNameEntry(taskEntry.GetType()));
            ImGui::TableNextColumn();
            ImGui::Text("%u", taskEntry.GetHandle().Key);
        }

        ImGui::EndTable();
    }
}
