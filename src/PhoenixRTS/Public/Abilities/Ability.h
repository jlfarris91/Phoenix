#pragma once

#include "DLLExport.h"
#include "Platform.h"
#include "Component.h"
#include "Orders/Orders.h"

namespace Phoenix::RTS
{
    struct Action;

    struct PHOENIX_RTS_API IAbilityComponent : ECS::IComponent
    {
        PHX_DECLARE_INTERFACE_BEGIN(IAbilityComponent)
            PHX_REGISTER_BASE(IComponent)
        PHX_DECLARE_INTERFACE_END()
    };

    class PHOENIX_RTS_API IAbility : TSharedAsThis<IAbility>
    {
        PHX_DECLARE_INTERFACE(IAbility)

    public:

        virtual FName GetAbilityId() const = 0;

        virtual uint32 HandleOrder(EOrderType type, const Order& order) = 0;

        virtual uint32 GetPriority(const Order& order) = 0;

        virtual uint32 Acquire(const Order& order) = 0;

        virtual bool SupportsMagicBox(const Order& order) = 0;
    };

    class PHOENIX_RTS_API AbilityBase : IAbility
    {
        PHX_DECLARE_DERIVED_TYPE_BEGIN(AbilityBase, IAbility)
        PHX_DECLARE_DERIVED_TYPE_END()
        
        AbilityBase(const FName& abilityId);

        FName GetAbilityId() const override;

    protected:

        FName AbilityId;
    };
}
