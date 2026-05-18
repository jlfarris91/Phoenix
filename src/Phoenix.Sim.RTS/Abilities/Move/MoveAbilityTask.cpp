#include "MoveAbilityTask.h"

#include "MoveAbilityHandler.h"
#include "Phoenix.Sim.RTS/Orders/FeatureOrders.h"
#include "Phoenix.Sim.RTS/Orders/Orders.h"
#include "Phoenix.Sim.Steering/FeatureSteering.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::Tasks;
using namespace Phoenix::RTS;

void MoveAbilityTask::OnCreate(WorldRef world, uint32 context)
{
    SetIntervalTicks(world, 1);
}

void MoveAbilityTask::OnUpdate(WorldRef world, uint32 context)
{
    UnitId unit = UnitId(context);
    
    if (ActiveState == EMoveAbilityState::Idle)
    {
        return;
    }

    AbilityStateResult result = EAbilityStateResult::Fail;

    switch (ActiveState)
    {
        case EMoveAbilityState::MoveToPosition: result = States.MoveToPosition.Update(world, unit); break;
        case EMoveAbilityState::FollowEntity:   result = States.FollowEntity.Update(world, unit); break;
        default:                                break;
    }

    if (result != EAbilityStateResult::Continue)
    {
        FeatureOrders::StaticOnActiveOrderCompleted(world, unit, result == EAbilityStateResult::Complete);
    }
}

void MoveAbilityTask::OnFinish(WorldRef world, uint32 context)
{
    ExitActiveState(world, UnitId(context));
}

bool MoveAbilityTask::ExecuteOrder(WorldRef world, UnitId unit, const Order& order)
{
    if (order.OrderIndex == MoveAbilityHandler::Commands::Patrol)
    {
        // TODO (jfarris): implement patrol states
    }
    else 
    {
        if (order.TargetEntity != EntityId::Invalid && FeatureECS::IsEntityValid(world, order.TargetEntity))
        {
            ActiveState = EMoveAbilityState::FollowEntity;
            auto result = States.FollowEntity.Enter(world, unit, order.TargetEntity, 0);
            return result != EAbilityStateResult::Fail;
        }

        ActiveState = EMoveAbilityState::MoveToPosition;
        auto result = States.MoveToPosition.Enter(world, unit, order.TargetLocation, 0);
        return result != EAbilityStateResult::Fail;
    }

    return false;
}

void MoveAbilityTask::Interrupt(WorldRef world, UnitId unit)
{
    switch (ActiveState)
    {
        case EMoveAbilityState::MoveToPosition: States.MoveToPosition.Interrupt(world, unit); break;
        case EMoveAbilityState::FollowEntity:   States.FollowEntity.Interrupt(world, unit); break;
        default:                                break;
    }

    ExitActiveState(world, unit);
}

void MoveAbilityTask::ExitActiveState(WorldRef world, UnitId unit)
{
    switch (ActiveState)
    {
        case EMoveAbilityState::MoveToPosition: States.MoveToPosition.Exit(world, unit); break;
        case EMoveAbilityState::FollowEntity:   States.FollowEntity.Exit(world, unit); break;
        default:                                break;
    }

    ActiveState = EMoveAbilityState::Idle;
    Steering::FeatureSteering::Stop(world, unit);
}
