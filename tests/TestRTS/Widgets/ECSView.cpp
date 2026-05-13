#include "ECSView.h"

#include <imgui.h>

#include "../imgui/ImGuiPropertyGrid.h"
#include "PhoenixSim/Session.h"

#include "PhoenixSim/ECS/FeatureECS.h"

using namespace Phoenix;
using namespace Phoenix::ECS;

void ECSView::Render(SessionConstRef session, WorldConstRef world)
{
    const FeatureECSDynamicBlock* ecsDynamicBlock = world.GetBlock<FeatureECSDynamicBlock>();
    if (!ecsDynamicBlock)
    {
        ImGui::Text("World does not have the ECS feature");
        return;
    }

    if (ImGui::BeginTable("Stats", 2, ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableNextColumn();
        ImGui::Text("Num Entities:");
        ImGui::TableNextColumn();
        ImGui::Text("%u", ecsDynamicBlock->Entities.GetNumActive());

        ImGui::TableNextColumn();
        ImGui::Text("Entities HWM:");
        ImGui::TableNextColumn();
        ImGui::Text("%u", ecsDynamicBlock->Entities.GetNumHighWaterMark());

        ImGui::TableNextColumn();
        ImGui::Text("Num Tags:");
        ImGui::TableNextColumn();
        ImGui::Text("%u", ecsDynamicBlock->Tags.GetNumValidTags());

        ImGui::TableNextColumn();
        ImGui::Text("Num Group Pairs:");
        ImGui::TableNextColumn();
        ImGui::Text("%u", ecsDynamicBlock->Groups.GetNumValidPairs());

        ImGui::EndTable();
    }

    if (ImGui::TreeNode("Systems:"))
    {
        RenderSystems(session);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Archetypes:"))
    {
        RenderArchetypes(world);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Entities:"))
    {
        RenderEntities(world);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Tags:"))
    {
        RenderTags(world);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Groups:"))
    {
        RenderGroups(world);
        ImGui::TreePop();
    }
}

void ECSView::RenderSystems(SessionConstRef session)
{
    std::shared_ptr<FeatureECS> featureECS = session.GetFeatureSet()->GetFeature<FeatureECS>();
    if (!featureECS)
    {
        return;
    }

    for (const std::shared_ptr<ISystem>& system : featureECS->GetSystems())
    {
        const auto& systemDescriptor = system->GetTypeDescriptor();
        if (ImGui::CollapsingHeader(systemDescriptor.GetDisplayName().c_str()))
        {
            DrawPropertyGrid(system.get(), systemDescriptor);
        }
    }
}

void ECSView::RenderArchetypes(WorldConstRef world)
{
    const FeatureECSDynamicBlock* ecsDynamicBlockPtr = world.GetBlock<FeatureECSDynamicBlock>();
    if (!ecsDynamicBlockPtr)
    {
        return;
    }

    const auto& archetypeManager = ecsDynamicBlockPtr->ArchetypeManager;

    if (ImGui::BeginTable("Props", 2, ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableNextColumn();
        ImGui::Text("Num Active:");
        ImGui::TableNextColumn();
        ImGui::Text("%u", archetypeManager.GetNumActiveArchetypes());

        ImGui::TableNextColumn();
        ImGui::Text("Num Lists:");
        ImGui::TableNextColumn();
        ImGui::Text("%u", archetypeManager.GetNumArchetypeLists());
                                
        ImGui::EndTable();
    }

    if (ImGui::TreeNode("Definitions:"))
    {
        uint32 index = 0;
        for (auto && [id, archDef] : archetypeManager.GetArchetypeDefinitions())
        {
            char treeNodeId[64];
            (void)snprintf(treeNodeId, sizeof(treeNodeId), "[%u] %u", index, (hash32_t)id);

            if (ImGui::TreeNode(treeNodeId))
            {
                if (ImGui::BeginTable("Props", 2, ImGuiTableFlags_SizingFixedFit))
                {
                    ImGui::TableNextColumn();
                    ImGui::Text("Num Comps:");
                    ImGui::TableNextColumn();
                    ImGui::Text("%u", archDef.GetNumComponents());

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("Total Size:");
                    ImGui::TableNextColumn();
                    ImGui::Text("%u", archDef.GetTotalSize());
                                
                    ImGui::EndTable();
                }

                if (ImGui::TreeNode("Components:"))
                {
                    for (auto i = 0; i < archDef.GetNumComponents(); ++i)
                    {
                        const ComponentDefinition& compDef = archDef[i];
                                    
                        if (ImGui::TreeNode(compDef.TypeDescriptor->GetDisplayName().c_str()))
                        {
                            if (ImGui::BeginTable("Props", 2, ImGuiTableFlags_SizingFixedFit))
                            {                                
                                ImGui::EndTable();
                            }
                                        
                            ImGui::TreePop();
                        }
                    }

                    ImGui::TreePop();
                }
                            
                ImGui::TreePop();
            }
                        
            ++index;
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Chunks:"))
    {
        uint32 listIndex = 0;

        archetypeManager.ForEachArchetypeList([&listIndex](const FixedArchetypeList& list)
        {
            char treeNodeId[64];
            (void)snprintf(treeNodeId, sizeof(treeNodeId), "[%u] %u (%u)", listIndex, list.GetId(), (hash32_t)list.GetDefinition().GetId());

            if (ImGui::TreeNode(treeNodeId))
            {
                if (ImGui::BeginTable("Props", 2, ImGuiTableFlags_SizingFixedFit))
                {
                    ImGui::TableNextColumn();
                    ImGui::Text("Instances:");
                    ImGui::TableNextColumn();
                    ImGui::Text("%u / %u", list.GetNumActiveInstances(), list.GetInstanceCapacity());

                    ImGui::TableNextColumn();
                    ImGui::Text("Size:");
                    ImGui::TableNextColumn();
                    ImGui::Text("%u / %u", list.GetSize(), list.GetCapacity());

                    ImGui::EndTable();
                }

                if (ImGui::TreeNode("Instances:"))
                {
                    uint32 instanceIndex = 0;
                    list.ForEachInstance([&](const ArchetypeHandle& handle)
                    {
                        char instanceTreeNodeId[64];
                        (void)snprintf(instanceTreeNodeId, sizeof(instanceTreeNodeId), "[%u] %u", instanceIndex, (hash32_t)handle.GetEntityId());

                        if (ImGui::TreeNode(instanceTreeNodeId))
                        {
                            list.ForEachComponent(handle, [](const ComponentDefinition& compDef, const void* comp)
                            {
                                if (compDef.TypeDescriptor && ImGui::TreeNode(compDef.TypeDescriptor->GetDisplayName().c_str()))
                                {
                                    DrawPropertyGrid(comp, *compDef.TypeDescriptor);
                                    ImGui::TreePop();
                                }
                            });

                            ImGui::TreePop();
                        }

                        ++instanceIndex;
                    });

                    ImGui::TreePop();
                }

                ImGui::TreePop();
            }

            ++listIndex;
        });

        ImGui::TreePop();
    }
}

void ECSView::RenderEntities(WorldConstRef world)
{
    if (ImGui::BeginTable("Entities", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("Index");
        ImGui::TableSetupColumn("Id");
        ImGui::TableSetupColumn("Kind");
        ImGui::TableHeadersRow();

        if (auto entitiesPtr = FeatureECS::GetEntities(world))
        {
            auto& entities = *entitiesPtr;
            entities.ForEach([&](const Entity& entity)
            {
                ImGui::TableNextColumn();
                ImGui::Text("%u:", entities.GetEntityIndex(entity.Id));
                ImGui::TableNextColumn();
                ImGui::Text("%u", (uint32)entity.Id);
                ImGui::TableNextColumn();
                ImGui::Text("%s", FName::GetNameEntry(entity.Kind));
            });
        }

        ImGui::EndTable();
    }
}

void ECSView::RenderTags(WorldConstRef world)
{
    if (ImGui::BeginTable("Tags", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("Entity");
        ImGui::TableSetupColumn("Tag");
        ImGui::TableHeadersRow();

        if (auto tagsPtr = FeatureECS::GetTags(world))
        {
            tagsPtr->ForEach([](const EntityTag& entityTag)
            {
                ImGui::TableNextColumn();
                ImGui::Text("%u:", (uint32)entityTag.Entity);
                ImGui::TableNextColumn();
                ImGui::Text("%s", FName::GetNameEntry(entityTag.Tag));
            });
        }

        ImGui::EndTable();
    }
}

void ECSView::RenderGroups(WorldConstRef world)
{
    if (ImGui::BeginTable("Groups", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("Group");
        ImGui::TableSetupColumn("Entity");
        ImGui::TableHeadersRow();

        if (auto groupsPtr = FeatureECS::GetGroups(world))
        {
            groupsPtr->ForEach([](const GroupEntity& groupEntity)
            {
                ImGui::TableNextColumn();
                ImGui::Text("%u", (uint32)groupEntity.Group);
                ImGui::TableNextColumn();
                ImGui::Text("%u", (uint32)groupEntity.Entity);
            });
        }

        ImGui::EndTable();
    }
}
