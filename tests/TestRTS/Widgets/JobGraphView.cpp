#include "JobGraphView.h"

#include <imgui.h>

#include "PhoenixSim/ECS/FeatureECS.h"

using namespace Phoenix;
using namespace Phoenix::ECS;

void JobGraphView::Render(SessionConstRef session, WorldConstRef world)
{
    if (ImGui::BeginTabBar("##phases"))
    {
        if (ImGui::BeginTabItem("PreUpdate"))
        {
            JobGraphPreUpdate.Draw(FeatureECS::GetScheduler(world, EJobPhase::PreUpdate));
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Update"))
        {
            JobGraphUpdate.Draw(FeatureECS::GetScheduler(world, EJobPhase::Update));
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("PostUpdate"))
        {
            JobGraphPostUpdate.Draw(FeatureECS::GetScheduler(world, EJobPhase::PostUpdate));
            ImGui::EndTabItem();
        }

        for (auto& [name, sched] : FeatureECS::GetNamedSchedulers(world))
        {
            const char* label = FName::GetNameEntry(name);
            if (!label || label[0] == '\0') continue;
            if (ImGui::BeginTabItem(label))
            {
                NamedJobGraphPanels[(uint32_t)name].Draw(*sched);
                ImGui::EndTabItem();
            }
        }

        ImGui::EndTabBar();
    }
}
