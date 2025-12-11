
#pragma once

#include "DLLExport.h"
#include "Features.h"

#ifndef PHX_RTS_ORDER_QUEUE_MAX_ORDERS
#define PHX_RTS_ORDER_QUEUE_MAX_ORDERS 4096
#endif

namespace Phoenix::RTS
{
    class IAbility;
}

namespace Phoenix::ECS
{
    struct EntityId;
}

namespace Phoenix::RTS
{
    struct UnitId;
    struct Order;

    struct FeatureAbilitiesDynamicBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_DYNAMIC(FeatureAbilitiesDynamicBlock)
    };

    class FeatureAbilities : public IFeature
    {
        PHX_FEATURE_BEGIN(FeatureAbilities)
            FEATURE_WORLD_BLOCK(FeatureAbilitiesDynamicBlock)
            FEATURE_CHANNEL(FeatureChannels::HandleWorldAction)
            FEATURE_CHANNEL(FeatureChannels::PostWorldUpdate)
        PHX_FEATURE_END()

    public:

        bool RegisterAbility(const TSharedPtr<IAbility>& ability);

        TSharedPtr<IAbility> GetAbility(const FName& abilityId) const;

        static TSharedPtr<IAbility> StaticGetAbility(WorldConstRef world, const FName& abilityId);

        static bool AddAbility(WorldRef world, const UnitId& unit, const FName& abilityId);

        static bool RemoveAbility(WorldRef world, const UnitId& unit, const FName& abilityId);

        static bool HasAbility(WorldConstRef world, const UnitId& unit, const FName& abilityId);

        static bool AddAbilitiesFromData(WorldRef world, const UnitId& unit, const FName& unitData);

    protected:

        void Initialize() override;
        void Shutdown() override;

        void OnWorldInitialize(WorldRef world) override;
        void OnWorldShutdown(WorldRef world) override;

        TMap<FName, TSharedPtr<IAbility>> Abilities;
    };
}
