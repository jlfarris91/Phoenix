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
    struct Action;

    struct IAbilityComponent : ECS::IComponent
    {
        PHX_DECLARE_INTERFACE_BEGIN(IAbilityComponent)
            PHX_REGISTER_BASE(IComponent)
        PHX_DECLARE_INTERFACE_END()
    };

    class IAbility : TSharedAsThis<IAbility>
    {
        PHX_DECLARE_INTERFACE(IAbility)

    public:

        virtual FName GetAbilityId() const = 0;

        virtual void Initialize(SessionRef session) = 0;

        virtual void Shutdown(SessionRef session) = 0;

        virtual void OnWorldInitialize(WorldRef world) = 0;

        virtual void OnWorldShutdown(WorldRef world) = 0;

        virtual bool AddAbility(WorldRef world, const UnitId& unit) = 0;

        virtual bool RemoveAbility(WorldRef world, const UnitId& unit) = 0;

        virtual bool HasAbility(WorldConstRef world, const UnitId& unit) = 0;

        virtual uint32 HandleOrder(EOrderType type, const Order& order) = 0;

        virtual uint32 GetPriority(const Order& order) = 0;

        virtual uint32 Acquire(const Order& order) = 0;

        virtual bool SupportsMagicBox(const Order& order) = 0;
    };

    class AbilityBase : public IAbility
    {
        PHX_DECLARE_DERIVED_TYPE_BEGIN(AbilityBase, IAbility)
        PHX_DECLARE_DERIVED_TYPE_END()
        
        AbilityBase(const FName& abilityId);

        FName GetAbilityId() const override;

    protected:

        FName AbilityId;
    };
}
