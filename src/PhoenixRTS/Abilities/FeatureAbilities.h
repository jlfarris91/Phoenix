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
        {
            FEATURE_CHANNEL(FeatureChannels::HandleWorldAction)
            FEATURE_CHANNEL(FeatureChannels::WorldInitialize)
            FEATURE_CHANNEL(FeatureChannels::WorldShutdown)
        }

    public:

        //
        // Ability Handlers
        //

        void RegisterAbilityHandler(const std::shared_ptr<IAbilityHandler>& handler);

        template <class T, class ...TArgs>
        std::shared_ptr<T> RegisterAbilityHandler(TArgs&&... args)
        {
            auto handler = std::make_shared<T>(std::forward<TArgs>(args)...);
            RegisterAbilityHandler(handler);
            return handler;
        }

        bool UnregisterAbilityHandler(const FName& abilityId);

        std::shared_ptr<IAbilityHandler> FindAbilityHandlerCached(WorldConstRef world, const FName& abilityId);

        std::shared_ptr<IAbilityHandler> FindAbilityHandler(WorldConstRef world, const FName& abilityId) const;

        static std::shared_ptr<IAbilityHandler> StaticFindAbilityHandler(WorldConstRef world, const FName& abilityId);

        static bool AddAbility(WorldRef world, const UnitId& unit, const FName& abilityId);

        static bool RemoveAbility(WorldRef world, const UnitId& unit, const FName& abilityId);

        static bool HasAbility(WorldConstRef world, const UnitId& unit, const FName& abilityId);

        static bool AddAbilitiesFromData(WorldRef world, const UnitId& unit, const FName& unitData);

        static uint32 GetAbilities(WorldConstRef world, const UnitId& unit, std::vector<FName>& outAbilityIds);

    protected:

        void Initialize(const std::shared_ptr<Phoenix::Session>& session) override;
        void Shutdown() override;

        std::unordered_map<FName, std::shared_ptr<IAbilityHandler>> AbilityIdToHandlerMap;
    };
}
