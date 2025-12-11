#pragma once

#include "DLLExport.h"
#include "Platform.h"
#include "Component.h"
#include "Session.h"
#include "Worlds.h"
#include "Orders/Orders.h"
#include "Units/FeatureUnit.h"

namespace Phoenix::RTS
{
    class FeatureAbilities;
}

namespace Phoenix::RTS
{
    struct Action;

    struct IAbilityComponent : ECS::IComponent
    {
        PHX_DECLARE_INTERFACE_BEGIN(IAbilityComponent)
            PHX_REGISTER_BASE(IComponent)
        PHX_DECLARE_INTERFACE_END()
    };

    struct AbilityPriority
    {
        static uint32 All();
        static uint32 SelfTargetOrAll(UnitId unit, ECS::EntityId target);
        static uint32 Closest(WorldConstRef world, UnitId unit, const Vec2& target);
    };

    class IAbility : TSharedAsThis<IAbility>
    {
        PHX_DECLARE_INTERFACE(IAbility)

    public:

        virtual FName GetAbilityId() const = 0;

        virtual void Initialize(SessionRef session);

        virtual void Shutdown(SessionRef session);

        virtual void OnWorldInitialize(WorldRef world);

        virtual void OnWorldShutdown(WorldRef world);

        virtual bool AddAbility(WorldRef world, const UnitId& unit) const;

        virtual bool RemoveAbility(WorldRef world, const UnitId& unit) const;

        virtual bool HasAbility(WorldConstRef world, const UnitId& unit) const;

        virtual uint32 GetCommandPriority(WorldRef world, UnitId unit, const Command& command) const;

        virtual bool IsTransient(WorldRef world, const FName& abilityId) const;

        virtual bool ExecuteOrder(WorldRef world, UnitId unit, const Order& order) const;

        virtual bool InterruptOrder(WorldRef world, UnitId unit, const Order& order) const;

        virtual uint32 Acquire(const Order& order) const;

        virtual bool SupportsMagicBox(const Order& order) const;
    };

    class AbilityBase : public IAbility
    {
        PHX_DECLARE_DERIVED_TYPE_BEGIN(AbilityBase, IAbility)
        PHX_DECLARE_DERIVED_TYPE_END()
        
        AbilityBase(const FName& abilityId);

        void Initialize(SessionRef session) override;
        void Shutdown(SessionRef session) override;

        FName GetAbilityId() const override;

    protected:

        TSharedPtr<FeatureAbilities> Abilities;
        FName AbilityId;
    };
}
