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

    enum class EMoveAbilityState
    {
        Idle,
        MoveToPosition
    };

    struct MoveAbilityComponent : IAbilityComponent
    {
        PHX_ECS_DECLARE_COMPONENT_BEGIN(MoveAbilityComponent)
        PHX_ECS_DECLARE_COMPONENT_END()

        MoveAbilityComponent();

        union
        {
            MoveToPositionState MoveToPosition;
        } ActiveState;

        EMoveAbilityState State = EMoveAbilityState::Idle;
    };

    class MoveAbility : public AbilityBase
    {
        PHX_DECLARE_DERIVED_TYPE_BEGIN(MoveAbility, AbilityBase)
        PHX_DECLARE_DERIVED_TYPE_END()

    public:

        struct Commands
        {
            static constexpr uint8 MoveToPosition = 0;
        };

        MoveAbility();

        void Initialize(SessionRef session) override;

        void Shutdown(SessionRef session) override;

        void OnWorldInitialize(WorldRef world) override;

        void OnWorldShutdown(WorldRef world) override;

        bool AddAbility(WorldRef world, const UnitId& unit) const override;

        bool RemoveAbility(WorldRef world, const UnitId& unit) const override;

        bool HasAbility(WorldConstRef world, const UnitId& unit) const override;

        uint32 GetCommandPriority(WorldRef world, UnitId unit, const Command& command) const override;

        bool ExecuteOrder(WorldRef world, UnitId unit, const Order& order) const override;

        bool InterruptOrder(WorldRef world, UnitId unit, const Order& order) const override;

        uint32 Acquire(const Order& order) const override;

        bool SupportsMagicBox(const Order& order) const override;

    private:

        TSharedPtr<MoveAbilitySystem> System;
    };
}
