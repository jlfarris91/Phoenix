
#pragma once

#include "PhoenixSim/Features.h"
#include "PhoenixRTS/DLLExport.h"
#include "PhoenixSim/Containers/Array.h"

namespace Phoenix::RTS
{
    struct Command;
    class IAbilityHandler;
}

namespace Phoenix::ECS
{
    struct EntityId;
}

namespace Phoenix::RTS
{
    struct UnitId;
    struct Order;

    class PHOENIX_RTS_API FeatureAbilities : public IFeature
    {
        PHX_FEATURE_BEGIN(FeatureAbilities)
            FEATURE_CHANNEL(FeatureChannels::HandleWorldAction)
            FEATURE_CHANNEL(FeatureChannels::WorldInitialize)
            FEATURE_CHANNEL(FeatureChannels::WorldShutdown)
        PHX_FEATURE_END()

    public:

        //
        // Ability Handlers
        //

        void RegisterAbilityHandler(const TSharedPtr<IAbilityHandler>& handler);

        template <class T, class ...TArgs>
        TSharedPtr<T> RegisterAbilityHandler(TArgs&&... args)
        {
            TSharedPtr<T> handler = MakeShared<T>(std::forward<TArgs>(args)...);
            RegisterAbilityHandler(handler);
            return handler;
        }

        bool UnregisterAbilityHandler(const FName& abilityId);

        TSharedPtr<IAbilityHandler> FindAbilityHandlerCached(WorldConstRef world, const FName& abilityId);

        TSharedPtr<IAbilityHandler> FindAbilityHandler(WorldConstRef world, const FName& abilityId) const;

        static TSharedPtr<IAbilityHandler> StaticFindAbilityHandler(WorldConstRef world, const FName& abilityId);

        static bool AddAbility(WorldRef world, const UnitId& unit, const FName& abilityId);

        static bool RemoveAbility(WorldRef world, const UnitId& unit, const FName& abilityId);

        static bool HasAbility(WorldConstRef world, const UnitId& unit, const FName& abilityId);

        static bool AddAbilitiesFromData(WorldRef world, const UnitId& unit, const FName& unitData);

        static uint32 GetAbilities(WorldConstRef world, const UnitId& unit, TArray2<FName>& outAbilityIds);

        //
        // Order Handling
        //

        static bool StaticHandleOrder(WorldRef world, const UnitId& unit, const Order& order);

        bool HandleOrder(WorldRef world, const UnitId& unit, const Order& order) const;

        static void StaticOnActiveOrderCompleted(WorldRef world, const UnitId& unit, bool success);

        void OnActiveOrderCompleted(WorldRef world, const UnitId& unit, bool success) const;

    protected:

        void Initialize() override;
        void Shutdown() override;

        bool OnHandleWorldAction(WorldRef world, const FeatureActionArgs& args) override;
        void OnWorldInitialize(WorldRef world) override;
        void OnWorldShutdown(WorldRef world) override;

        bool ExitActiveAbility(WorldRef world, UnitId unit) const;

        bool HandleCommand(WorldRef world, const Command& command);
        bool HandleSmartCommand(WorldRef world, const Command& command);

        bool HandleCommand(WorldRef world, const UnitId& unit, const Command& command) const;

        TMap<FName, TSharedPtr<IAbilityHandler>> AbilityIdToHandlerMap;
    };
}
