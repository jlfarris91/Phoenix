#pragma once

#include "Ability.h"

namespace Phoenix::RTS
{
    struct Action;

    struct PHOENIX_RTS_API MoveAbilityComponent : IAbilityComponent
    {
    };

    class PHOENIX_RTS_API MoveAbility : AbilityBase
    {
        PHX_DECLARE_DERIVED_TYPE_BEGIN(MoveAbility, AbilityBase)
        PHX_DECLARE_DERIVED_TYPE_END()

    public:
        
        MoveAbility();
        
        uint32 HandleOrder(EOrderType type, const Order& order) override;

        uint32 GetPriority(const Order& order) override;

        uint32 Acquire(const Order& order) override;

        bool SupportsMagicBox(const Order& order) override;
    };
}
