#pragma once

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Session.h"
#include "PhoenixSim/Worlds.h"
#include "PhoenixSim/ECS/Component.h"
#include "PhoenixSim/FixedPoint/FixedVector.h"

#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Orders/Orders.h"
#include "PhoenixRTS/Units/UnitId.h"

namespace Phoenix::LDS
{
    class ILDSQueryContext;
}

namespace Phoenix::ECS
{
    struct EntityId;
}

namespace Phoenix::RTS
{
    struct Command;
    struct UnitId;
    class FeatureAbilities;

    struct PHOENIX_RTS_API IAbilityComponent : ECS::IComponent
    {
        PHX_DECLARE_INTERFACE_BEGIN(IAbilityComponent)
            PHX_REGISTER_BASE(IComponent)
        PHX_DECLARE_INTERFACE_END()
    };

    struct PHOENIX_RTS_API AbilityPriority
    {
        static uint32 All();
        static uint32 SelfTargetOrAll(const UnitId& unit, ECS::EntityId target);
        static uint32 Closest(WorldConstRef world, const UnitId& unit, const Vec2& target);
    };

    struct PHOENIX_RTS_API AbilityCommandContext
    {
        UnitId Unit;
        FName AbilityId;
        FName SelectionGroupId;
        TSharedPtr<const LDS::ILDSQueryContext> LdsQueryContext;
    };

    class PHOENIX_RTS_API IAbilityHandler : TSharedAsThis<IAbilityHandler>
    {
        PHX_DECLARE_INTERFACE(IAbilityHandler)

    public:

        virtual FName GetAbilityId() const = 0;

        virtual void Initialize(SessionRef session);

        virtual void Shutdown(SessionRef session);

        virtual void OnWorldInitialize(WorldRef world);

        virtual void OnWorldShutdown(WorldRef world);

        virtual bool AddAbility(WorldRef world, const UnitId& unit) const;

        virtual bool RemoveAbility(WorldRef world, const UnitId& unit) const;

        virtual bool HasAbility(WorldConstRef world, const UnitId& unit) const;

        virtual bool IgnoreCommand(WorldConstRef world, const AbilityCommandContext& context, const Command& command) const;

        virtual uint32 GetCommandPriority(WorldConstRef world, const AbilityCommandContext& context, const Command& command) const;

        virtual uint32 GetSmartCommandPriority(WorldConstRef world, const AbilityCommandContext& context, const Command& command) const;

        virtual bool IsTransient(WorldConstRef world, const FName& abilityId) const;

        virtual bool ExecuteOrder(WorldRef world, const UnitId& unit, const Order& order) const;

        virtual bool InterruptOrder(WorldRef world, const UnitId& unit, const Order& order) const;

        virtual uint32 Acquire(const Order& order) const;

        virtual bool SupportsMagicBox(const Order& order) const;
    };

    class PHOENIX_RTS_API AbilityHandlerBase : public IAbilityHandler
    {
        PHX_DECLARE_DERIVED_TYPE_BEGIN(AbilityHandlerBase, IAbilityHandler)
        PHX_DECLARE_DERIVED_TYPE_END()
        
        AbilityHandlerBase(const FName& abilityId);

        void Initialize(SessionRef session) override;
        void Shutdown(SessionRef session) override;

        FName GetAbilityId() const override;

    protected:

        TSharedPtr<FeatureAbilities> Abilities;
        FName AbilityId;
    };
}
