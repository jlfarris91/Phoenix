
#pragma once

#include "DLLExport.h"
#include "Features.h"
#include "FixedAbilities.h"

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
    struct Unit;
    struct Order;
    
    struct PHOENIX_RTS_API FeatureAbilitiesDynamicBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_DYNAMIC(FeatureAbilitiesDynamicBlock)
    };

    class PHOENIX_RTS_API FeatureAbilities : public IFeature
    {
        PHX_FEATURE_BEGIN(FeatureAbilities)
            FEATURE_WORLD_BLOCK(FeatureAbilitiesDynamicBlock)
            FEATURE_CHANNEL(FeatureChannels::HandleWorldAction)
            FEATURE_CHANNEL(FeatureChannels::PostWorldUpdate)
        PHX_FEATURE_END()

    public:

        static bool AddAbility(WorldRef world, const Unit& unit, const FName& abilityId);
        
        static bool RemoveAbility(WorldRef world, const Unit& unit, const FName& abilityId);
        
        static bool HasAbility(WorldConstRef world, const Unit& unit, const FName& abilityId);

        static bool AddAbilitiesFromData(WorldRef world, const Unit& unit, const FName& unitData);

        bool RegisterAbility(const TSharedPtr<IAbility>& ability);

    protected:

        void OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;

        static void SortAbilities(WorldRef world);

        TMap<FName, TSharedPtr<IAbility>> Abilities;
    };
}
