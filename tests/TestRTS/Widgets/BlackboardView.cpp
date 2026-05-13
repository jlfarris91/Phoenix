#include "BlackboardView.h"

#include <imgui.h>

#include "PhoenixSim/Worlds.h"
#include "PhoenixSim/Blackboard/FeatureBlackboard.h"
#include "PhoenixSim/Blackboard/FixedBlackboard.h"

using namespace Phoenix::Blackboard;

void BlackboardView::Render(Phoenix::SessionConstRef session, Phoenix::WorldConstRef world)
{
    const FeatureBlackboardBlock& blackboardBlock = world.GetBlockRef<FeatureBlackboardBlock>();

    if (ImGui::BeginTable("Stats", 2, ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableNextColumn();
        ImGui::Text("Num KVPs:");
        ImGui::TableNextColumn();
        ImGui::Text("%u", blackboardBlock.Blackboard.GetNumValidItems());

        ImGui::TableNextColumn();
        ImGui::Text("KVP HWM:");
        ImGui::TableNextColumn();
        ImGui::Text("%u", blackboardBlock.Blackboard.GetNum());

        ImGui::EndTable();
    }

    if (ImGui::BeginTable("KVPs", 4, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("Key (Hi)");
        ImGui::TableSetupColumn("Key (Lo)");
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();
                
        for (const auto& blackboardItem : blackboardBlock.Blackboard)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%u", BlackboardKey::GetKeyHi(blackboardItem.Key));
            ImGui::TableNextColumn();
            ImGui::Text("%u", BlackboardKey::GetKeyLo(blackboardItem.Key));
            ImGui::TableNextColumn();
            ImGui::Text("%u", BlackboardKey::GetKeyType(blackboardItem.Key));
            ImGui::TableNextColumn();
            ImGui::Text("%llill", blackboardItem.Value);
        }

        ImGui::EndTable();
    }
}
