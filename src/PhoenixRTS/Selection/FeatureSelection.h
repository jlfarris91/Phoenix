
#pragma once

#include "PhoenixSim/Features.h"
#include "PhoenixSim/Containers/FixedArray.h"
#include "PhoenixSim/ECS/EntityId.h"

#include "PhoenixRTS/DLLExport.h"

#ifndef PHX_RTS_MAX_PLAYER_SELECTION_GROUPS_PER_PLAYER
#define PHX_RTS_MAX_PLAYER_SELECTION_GROUPS_PER_PLAYER 4
#endif

// TODO (jfarris): where do we define the max number of players? for now use 32
#define PHX_RTS_MAX_PLAYER_SELECTION_GROUPS (PHX_RTS_MAX_PLAYER_SELECTION_GROUPS_PER_PLAYER * 32)

namespace Phoenix::RTS
{
    struct Order;

    struct PHOENIX_RTS_API PlayerSelectionGroup
    {
        uint32 Player = 0;
        FName GroupName;
        ECS::EntityId GroupId;
    };

    struct PHOENIX_RTS_API FeatureSelectionDynamicBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_DYNAMIC(FeatureSelectionDynamicBlock)

        TFixedArray<PlayerSelectionGroup, PHX_RTS_MAX_PLAYER_SELECTION_GROUPS> PlayerSelectionGroups;
    };

    class PHOENIX_RTS_API FeatureSelection : public IFeature
    {
        PHX_DECLARE_FEATURE_TYPE(FeatureSelection)

    public:

        FeatureSelection();

        static constexpr uint32 MaxNumGroupsPerPlayer = PHX_RTS_MAX_PLAYER_SELECTION_GROUPS_PER_PLAYER;

        static ECS::EntityId GetOrCreatePlayerSelection(WorldRef world, uint32 player, const FName& name);

        static ECS::EntityId GetPlayerSelection(WorldConstRef world, uint32 player, const FName& name = FName::None);

        static bool AddToPlayerSelection(WorldRef world, uint32 player, const ECS::EntityId& entity, const FName& name = FName::None);

        static bool RemoveFromPlayerSelection(WorldRef world, uint32 player, const ECS::EntityId& entity, const FName& name = FName::None);

        static bool ClearPlayerSelection(WorldRef world, uint32 player, const FName& name = FName::None);

        static uint32 GetNumSelected(WorldConstRef world, uint32 player, const FName& name = FName::None);

    private:

        bool OnHandleWorldAction(WorldRef world, const FeatureActionArgs& args) override;
    };
}
