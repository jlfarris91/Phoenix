#pragma once

#include "Ability.h"
#include "AbilityStates.h"
#include "System.h"

namespace Phoenix::RTS
{
    struct Action;

    class MoveAbilitySystem : public ECS::ISystem
    {
        PHX_ECS_DECLARE_SYSTEM_BEGIN(MoveAbilitySystem)
        PHX_ECS_DECLARE_SYSTEM_END()

        void OnWorldUpdate(WorldRef world, const ECS::SystemUpdateArgs& args) override;
    };

    struct MoveAbilityComponent : IAbilityComponent
    {
        PHX_ECS_DECLARE_COMPONENT_BEGIN(MoveAbilityComponent)
        PHX_ECS_DECLARE_COMPONENT_END()

        MoveAbilityComponent();

        enum class EMoveAbilityState
        {
            MoveToPosition
        };

        union
        {
            MoveToPositionState MoveToPosition;
        } ActiveState;

        EMoveAbilityState State;
    };

    class MoveAbility : public AbilityBase
    {
        PHX_DECLARE_DERIVED_TYPE_BEGIN(MoveAbility, AbilityBase)
        PHX_DECLARE_DERIVED_TYPE_END()

    public:

        MoveAbility();

        void Initialize(SessionRef session) override;

        void Shutdown(SessionRef session) override;

        void OnWorldInitialize(WorldRef world) override;

        void OnWorldShutdown(WorldRef world) override;

        bool AddAbility(WorldRef world, const UnitId& unit) override;

        bool RemoveAbility(WorldRef world, const UnitId& unit) override;

        bool HasAbility(WorldConstRef world, const UnitId& unit) override;

        uint32 HandleOrder(EOrderType type, const Order& order) override;

        uint32 GetPriority(const Order& order) override;

        uint32 Acquire(const Order& order) override;

        bool SupportsMagicBox(const Order& order) override;

    private:

        TSharedPtr<MoveAbilitySystem> System;
    };
}
