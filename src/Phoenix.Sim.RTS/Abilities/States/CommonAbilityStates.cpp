#include "PhoenixRTS/Abilities/States/CommonAbilityStates.h"

#include "PhoenixSteering/FeatureSteering.h"
#include "PhoenixRTS/Units/FeatureUnit.h"

using namespace Phoenix; 
using namespace Phoenix::ECS; 
using namespace Phoenix::Steering; 
using namespace Phoenix::RTS;

AbilityStateResult MoveToEntityState::Enter(
    WorldRef world,
    const EntityId& entity,
    const EntityId& target,
    Distance range)
{
    Target = target;
    Range = range;
    LastKnownPosition = Vec2::Max;

    if (!FeatureECS::IsEntityValid(world, Target))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::TargetLost };
    }

    if (entity == target || FeatureECS::IsInRange(world, entity, target, range))
    {
        return EAbilityStateResult::Complete;
    }

    if (!FeatureUnit::UnitCanMove(world, UnitId(entity)))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::CannotMove };
    }

    if (!FeatureSteering::FollowEntity(world, entity, target, range))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::FailedToMove };
    }

    LastKnownPosition = FeatureECS::GetWorldPosition(world, target);
    return EAbilityStateResult::Continue;
}

AbilityStateResult MoveToEntityState::Update(WorldRef world, const EntityId& entity)
{
    if (!FeatureECS::IsEntityValid(world, Target) || FeatureUnit::UnitIsDead(world, UnitId(Target)))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::TargetLost };
    }

    if (!FeatureUnit::UnitCanMove(world, UnitId(entity)))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::CannotMove };
    }

    if (FeatureECS::IsInRange(world, entity, Target, Range))
    {
        return EAbilityStateResult::Complete;
    }

    if (FeatureSteering::HasFinishedMoving(world, entity))
    {
        return EAbilityStateResult::Complete;
    }

    if (!FeatureSteering::IsMoving(world, entity))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::NoLongerSeekingGoal };
    }

    LastKnownPosition = FeatureECS::GetWorldPosition(world, Target);
    return EAbilityStateResult::Continue;
}

void MoveToEntityState::Interrupt(WorldRef world, const EntityId& entity)
{
    Exit(world, entity);
}

void MoveToEntityState::Exit(WorldRef world, const EntityId& entity)
{
    FeatureSteering::Stop(world, entity);
    LastKnownPosition = Vec2::Max;
}

AbilityStateResult MoveToLocationState::Enter(WorldRef world, const EntityId& entity, const Vec2& target, Distance range)
{
    Target = target;
    Range = range;

    if (FeatureECS::IsInRange(world, entity, target, range))
    {
        return EAbilityStateResult::Complete;
    }

    if (!FeatureUnit::UnitCanMove(world, UnitId(entity)))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::CannotMove };
    }

    if (!FeatureSteering::MoveToLocation(world, entity, target, range))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::FailedToMove };
    }

    return EAbilityStateResult::Continue;
}

AbilityStateResult MoveToLocationState::Update(WorldRef world, const EntityId& entity)
{
    if (FeatureUnit::UnitIsImmobilized(world, UnitId(entity)))
    {
        return EAbilityStateResult::Continue;
    }

    if (FeatureECS::IsInRange(world, entity, Target, Range))
    {
        return EAbilityStateResult::Complete;
    }

    if (FeatureSteering::HasFinishedMoving(world, entity))
    {
        return EAbilityStateResult::Complete;
    }

    if (!FeatureSteering::IsMoving(world, entity))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::NoLongerSeekingGoal };
    }

    return EAbilityStateResult::Continue;
}

void MoveToLocationState::Interrupt(WorldRef world, const EntityId& entity)
{
    Exit(world, entity);
}

void MoveToLocationState::Exit(WorldRef world, const EntityId& entity)
{
    FeatureSteering::Stop(world, entity);
}

AbilityStateResult FaceEntityState::Enter(
    WorldRef world,
    const EntityId& entity,
    const EntityId& target,
    Distance range,
    Angle threshold)
{
    Target = target;
    MaxRange = range;
    Threshold = threshold;
    LastKnownPosition = Vec2::Max;

    if (entity == target)
    {
        return EAbilityStateResult::Complete;
    }

    if (!FeatureECS::IsEntityValid(world, Target))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::TargetLost };
    }

    if (!FeatureECS::IsInRange(world, entity, Target, MaxRange))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::TargetOutOfRange };
    }

    if (FeatureECS::IsFacing(world, entity, Target, Threshold))
    {
        return EAbilityStateResult::Complete;
    }

    if (!FeatureSteering::TurnToFace(world, entity, target))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::FailedToTurn };
    }

    LastKnownPosition = FeatureECS::GetWorldPosition(world, Target);
    return EAbilityStateResult::Continue;
}

AbilityStateResult FaceEntityState::Update(WorldRef world, const EntityId& entity)
{
    if (!FeatureECS::IsEntityValid(world, Target))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::TargetLost };
    }

    if (!FeatureECS::IsInRange(world, entity, Target, MaxRange))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::TargetOutOfRange };
    }

    if (FeatureECS::IsFacing(world, entity, Target, Threshold))
    {
        return EAbilityStateResult::Complete;
    }

    if (FeatureSteering::HasFinishedTurning(world, entity))
    {
        return EAbilityStateResult::Complete;
    }

    if (!FeatureSteering::IsTurning(world, entity))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::NoLongerSeekingGoal };
    }

    LastKnownPosition = FeatureECS::GetWorldPosition(world, Target);
    return EAbilityStateResult::Continue;
}

void FaceEntityState::Interrupt(WorldRef world, const EntityId& entity)
{
    Exit(world, entity);
}

void FaceEntityState::Exit(WorldRef world, const EntityId& entity)
{
    FeatureSteering::Stop(world, entity);
    LastKnownPosition = Vec2::Max;
}

AbilityStateResult FaceLocationState::Enter(
    WorldRef world,
    const EntityId& entity,
    const Vec2& target,
    Distance range,
    Angle threshold)
{
    Target = target;
    MaxRange = range;
    Threshold = threshold;

    if (!FeatureECS::IsInRange(world, entity, Target, MaxRange))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::TargetOutOfRange };
    }

    if (FeatureECS::IsFacing(world, entity, Target, Threshold))
    {
        return EAbilityStateResult::Complete;
    }

    if (!FeatureSteering::TurnToFace(world, entity, target))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::FailedToTurn };
    }

    return EAbilityStateResult::Continue;
}

AbilityStateResult FaceLocationState::Update(WorldRef world, const EntityId& entity)
{
    if (!FeatureECS::IsInRange(world, entity, Target, MaxRange))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::TargetOutOfRange };
    }

    if (FeatureECS::IsFacing(world, entity, Target, Threshold))
    {
        return EAbilityStateResult::Complete;
    }

    if (FeatureSteering::HasFinishedTurning(world, entity))
    {
        return EAbilityStateResult::Complete;
    }

    if (!FeatureSteering::IsTurning(world, entity))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::NoLongerSeekingGoal };
    }

    return EAbilityStateResult::Continue;
}

void FaceLocationState::Interrupt(WorldRef world, const EntityId& entity)
{
    Exit(world, entity);
}

void FaceLocationState::Exit(WorldRef world, const EntityId& entity)
{
    FeatureSteering::Stop(world, entity);
}

AbilityStateResult FollowEntityState::Enter(
    WorldRef world,
    const EntityId& entity,
    const EntityId& target,
    Distance range)
{
    Target = target;
    FollowRange = range;
    LastKnownPosition = Vec2::Max;
    return Update(world, entity);
}

AbilityStateResult FollowEntityState::Update(WorldRef world, const EntityId& entity)
{
    if (SubState == ESubState::Moving)
    {
        if (!FeatureSteering::IsSeekingGoal(world, entity))
        {
            return EAbilityStateResult::Complete;
        }

        if (world.GetSimTime() > RangeCheckTime)
        {
            RangeCheckTime = world.GetSimTime() + 1.0;
            if (FeatureECS::IsInRange(world, entity, Target, FollowRange))
            {
                return SetSubState(world, entity, ESubState::Waiting);
            }
        }
    }

    if (SubState == ESubState::Waiting)
    {
        if (world.GetSimTime() > RangeCheckTime)
        {
            RangeCheckTime = world.GetSimTime() + 1.0;
            if (!FeatureECS::IsInRange(world, entity, Target, FollowRange))
            {
                return SetSubState(world, entity, ESubState::Moving);
            }
        }
    }

    LastKnownPosition = FeatureECS::GetWorldPosition(world, Target);
    return EAbilityStateResult::Continue;
}

void FollowEntityState::Interrupt(WorldRef world, const EntityId& entity)
{
    Exit(world, entity);
}

void FollowEntityState::Exit(WorldRef world, const EntityId& entity)
{
    FeatureSteering::Stop(world, entity);
    LastKnownPosition = Vec2::Max;
}

AbilityStateResult FollowEntityState::SetSubState(WorldRef world, const EntityId& entity, ESubState subState)
{
    if (SubState == subState)
    {
        return EAbilityStateResult::Continue;
    }

    SubState = subState;

    if (SubState == ESubState::Moving)
    {
        if (!FeatureSteering::FollowEntity(world, entity, Target, FollowRange))
        {
            return { EAbilityStateResult::Complete };
        }
    }
    else if (SubState == ESubState::Waiting)
    {
        FeatureSteering::Stop(world, entity);
        RangeCheckTime = world.GetSimTime() + 1.0;
    }

    LastKnownPosition = FeatureECS::GetWorldPosition(world, Target);
    return EAbilityStateResult::Continue;
}
