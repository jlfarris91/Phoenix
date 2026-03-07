
#pragma once

#include "AbilityHandler.h"
#include "PhoenixSim/Features.h"
#include "PhoenixRTS/DLLExport.h"

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
        PHX_DECLARE_FEATURE_TYPE(FeatureAbilities)

    public:

        FeatureAbilities();

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

        static uint32 GetAbilities(WorldConstRef world, const UnitId& unit, TVector<FName>& outAbilityIds);

    protected:

        void Initialize(const TSharedPtr<Phoenix::Session>& session) override;
        void Shutdown() override;

        std::unordered_map<FName, TSharedPtr<IAbilityHandler>> AbilityIdToHandlerMap;
    };
}
