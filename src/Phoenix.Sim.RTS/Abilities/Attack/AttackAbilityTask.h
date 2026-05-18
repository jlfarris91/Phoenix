#pragma once

#include "Phoenix.Sim.RTS/Abilities/Attack/AttackAbilityStates.h"
#include "Phoenix.Sim.Tasks/TaskBase.h"

namespace Phoenix::RTS
{
    struct Order;

    struct PHOENIX_RTS_API AttackAbilityTask : Tasks::TaskBase
    {
        PHX_DECLARE_TASK(AttackAbilityTask)

        union
        {
            AttackTargetState AttackEntity;
            AttackLocationState AttackLocation;
            AttackMoveState AttackMove;
            FollowEntityState FollowEntity;
        } States;

        EAttackAbilityState ActiveState = EAttackAbilityState::Idle;

        void OnCreate(WorldRef world, uint32 context);
        void OnUpdate(WorldRef world, uint32 context);
        void OnFinish(WorldRef world, uint32 context);

        bool ExecuteOrder(WorldRef world, UnitId unit, const Order& order);

        bool ExecuteAttackTargetOrder(
            WorldRef world,
            UnitId unit,
            UnitId target,
            const Data::AttackAbilityPtr& attackAbility);

        bool ExecuteAttackMoveOrder(
            WorldRef world,
            UnitId unit,
            const Vec2& target,
            const Data::AttackAbilityPtr& attackAbility);

        bool ExecuteAttackGroundOrder(
            WorldRef world,
            UnitId unit,
            const Vec2& target,
            const Data::AttackAbilityPtr& attackAbility);

        void Interrupt(WorldRef world, UnitId unit);

        void ExitActiveState(WorldRef world, UnitId unit);
    };
}
