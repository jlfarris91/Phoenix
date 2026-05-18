#pragma once

#include "Phoenix.Sim.RTS/Abilities/States/CommonAbilityStates.h"
#include "Phoenix.Sim.RTS/Units/UnitId.h"
#include "Phoenix.Sim.Tasks/TaskBase.h"

namespace Phoenix::RTS
{
    struct Order;

    struct PHOENIX_RTS_API MoveAbilityTask : Tasks::TaskBase
    {
        PHX_DECLARE_TASK(MoveAbilityTask)

        enum class EMoveAbilityState : uint8
        {
            Idle,
            MoveToPosition,
            FollowEntity
        };

        union
        {
            MoveToLocationState MoveToPosition;
            FollowEntityState FollowEntity;
        } States;

        EMoveAbilityState ActiveState = EMoveAbilityState::Idle;

        void OnCreate(WorldRef world, uint32 context);
        void OnUpdate(WorldRef world, uint32 context);
        void OnFinish(WorldRef world, uint32 context);

        bool ExecuteOrder(WorldRef world, UnitId unit, const Order& order);

        void Interrupt(WorldRef world, UnitId unit);

        void ExitActiveState(WorldRef world, UnitId unit);
    };
}
