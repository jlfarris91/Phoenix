#include "Phoenix.Sim.RTS/Selection/FeatureSelection.h"

#include "Phoenix.Sim/ECS/EntityId.h"
#include "Phoenix.Sim/ECS/FeatureECS.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

bool FeatureSelection::AddToPlayerSelection(
    WorldRef world,
    uint32 player,
    const EntityId& entity,
    const FName& name)
{
    EntityId group = GetOrCreatePlayerSelection(world, player, name);
    if (group == EntityId::Invalid)
    {
        return false;
    }
    return FeatureECS::AddEntityToGroup(world, group, entity);
}

bool FeatureSelection::RemoveFromPlayerSelection(
    WorldRef world,
    uint32 player,
    const EntityId& entity,
    const FName& name)
{
    EntityId group = GetPlayerSelection(world, player, name);
    if (group == EntityId::Invalid)
    {
        return false;
    }
    return FeatureECS::RemoveEntityFromGroup(world, group, entity);
}

bool FeatureSelection::ClearPlayerSelection(WorldRef world, uint32 player, const FName& name)
{
    EntityId group = GetPlayerSelection(world, player, name);
    if (group == EntityId::Invalid)
    {
        return false;
    }
    return FeatureECS::ClearGroup(world, group);
}

uint32 FeatureSelection::GetNumSelected(WorldConstRef world, uint32 player, const FName& name)
{
    EntityId group = GetPlayerSelection(world, player, name);
    return group != EntityId::Invalid ? FeatureECS::GetGroupSize(world, group) : 0;
}

EntityId FeatureSelection::GetOrCreatePlayerSelection(WorldRef world, uint32 player, const FName& name)
{
    EntityId group = GetPlayerSelection(world, player, name);
    if (group != EntityId::Invalid)
    {
        return group;
    }

    FeatureSelectionDynamicBlock& block = world.GetBlockRef<FeatureSelectionDynamicBlock>();

    uint32 freeIndex = Index<uint32>::None;
    for (uint32 i = 0; i < MaxNumGroupsPerPlayer; ++i)
    {
        uint32 index = player * MaxNumGroupsPerPlayer + i;
        PlayerSelectionGroup& selectionGroup = block.PlayerSelectionGroups[index];
        if (selectionGroup.GroupId == EntityId::Invalid)
        {
            freeIndex = index;
            break;
        }
    }

    if (freeIndex == Index<uint32>::None)
    {
        return EntityId::Invalid;
    }

    group = FeatureECS::StaticAcquireEntity(world, "player_selection"_n);

    PlayerSelectionGroup& selectionGroup = block.PlayerSelectionGroups[freeIndex];
    selectionGroup.Player = player;
    selectionGroup.GroupName = name;
    selectionGroup.GroupId = group;

    return group;
}

EntityId FeatureSelection::GetPlayerSelection(WorldConstRef world, uint32 player, const FName& name)
{
    const FeatureSelectionDynamicBlock& block = world.GetBlockRef<FeatureSelectionDynamicBlock>();

    for (uint32 i = 0; i < MaxNumGroupsPerPlayer; ++i)
    {
        uint32 index = player * MaxNumGroupsPerPlayer + i;
        const PlayerSelectionGroup& selectionGroup = block.PlayerSelectionGroups[index];
        if (selectionGroup.Player == player && selectionGroup.GroupName == name)
        {
            return selectionGroup.GroupId;
        }
    }

    return EntityId::Invalid;
}

bool FeatureSelection::OnHandleWorldAction(WorldRef world, const FeatureActionArgs& args)
{
    if (args.Action.Verb == "player_selection_clear"_n)
    {
        uint32 player = args.Action.Args[0].AsUInt32;
        FName groupId = args.Action.Args[1].AsName;
        ClearPlayerSelection(world, player, groupId);
    }

    if (args.Action.Verb == "player_selection_add"_n)
    {
        uint32 player = args.Action.Args[0].AsUInt32;
        FName groupId = args.Action.Args[1].AsName;
        EntityId entity = args.Action.Args[2].AsUInt32;
        AddToPlayerSelection(world, player, entity, groupId);
    }

    if (args.Action.Verb == "player_selection_remove"_n)
    {
        uint32 player = args.Action.Args[0].AsUInt32;
        FName groupId = args.Action.Args[1].AsName;
        EntityId entity = args.Action.Args[2].AsUInt32;
        RemoveFromPlayerSelection(world, player, entity, groupId);
    }

    return false;
}
