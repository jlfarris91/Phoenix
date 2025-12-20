#include "PhoenixRTS/Abilities/AbilityStates.h"

#include "PhoenixSteering/FeatureSteering.h"
#include "PhoenixRTS/Units/FeatureUnit.h"

using namespace Phoenix; 
using namespace Phoenix::ECS; 
using namespace Phoenix::Steering; 
using namespace Phoenix::RTS;

AbilityStateResult MoveToEntityState::OnEnter(
    WorldRef world,
    const UnitId& unit,
    const EntityId& target,
    Distance range)
{
    Target = target;
    Range = range;

    if (!FeatureECS::IsEntityValid(world, Target))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::TargetLost };
    }

    if (unit == target || FeatureECS::IsInRange(world, unit, target, range))
    {
        return EAbilityStateResult::Complete;
    }

    if (!FeatureUnit::UnitCanMove(world, unit))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::CannotMove };
    }

    if (!FeatureSteering::FollowEntity(world, unit, target))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::FailedToMove };
    }

    return EAbilityStateResult::Continue;
}

AbilityStateResult MoveToEntityState::OnUpdate(WorldRef world, const UnitId& unit)
{
    if (!FeatureECS::IsEntityValid(world, Target) || FeatureUnit::UnitIsDead(world, UnitId(Target)))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::TargetLost };
    }

    if (!FeatureUnit::UnitCanMove(world, unit))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::CannotMove };
    }

    if (FeatureECS::IsInRange(world, unit, Target, Range))
    {
        return EAbilityStateResult::Complete;
    }

    if (!FeatureSteering::IsMoving(world, unit))
    {
        if (FeatureSteering::HasFinishedMoving(world, unit))
        {
            return EAbilityStateResult::Complete;
        }

        return { EAbilityStateResult::Fail, AbilityStateReasons::NoLongerSeekingGoal };
    }

    return EAbilityStateResult::Continue;
}

void MoveToEntityState::OnInterrupt(WorldRef world, const UnitId& unit)
{
    OnExit(world, unit);
}

void MoveToEntityState::OnExit(WorldRef world, const UnitId& unit)
{
    FeatureSteering::Stop(world, unit);
}

AbilityStateResult MoveToLocationState::OnEnter(WorldRef world, const UnitId& unit, const Vec2& target, Distance range)
{
    Target = target;
    Range = range;

    if (FeatureECS::IsInRange(world, unit, target, range))
    {
        return EAbilityStateResult::Complete;
    }

    if (!FeatureUnit::UnitCanMove(world, unit))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::CannotMove };
    }

    if (!FeatureSteering::MoveToLocation(world, unit, target))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::FailedToMove };
    }

    return EAbilityStateResult::Continue;
}

AbilityStateResult MoveToLocationState::OnUpdate(WorldRef world, const UnitId& unit)
{
    if (FeatureUnit::UnitIsImmobilized(world, unit))
    {
        return EAbilityStateResult::Continue;
    }

    if (FeatureECS::IsInRange(world, unit, Target, Range))
    {
        return EAbilityStateResult::Complete;
    }

    if (!FeatureSteering::IsMoving(world, unit))
    {
        if (FeatureSteering::HasFinishedMoving(world, unit))
        {
            return EAbilityStateResult::Complete;
        }

        return { EAbilityStateResult::Fail, AbilityStateReasons::NoLongerSeekingGoal };
    }

    return EAbilityStateResult::Continue;
}

void MoveToLocationState::OnInterrupt(WorldRef world, const UnitId& unit)
{
    OnExit(world, unit);
}

void MoveToLocationState::OnExit(WorldRef world, const UnitId& unit)
{
    FeatureSteering::Stop(world, unit);
}

AbilityStateResult FaceEntityState::OnEnter(
    WorldRef world,
    const UnitId& unit,
    const EntityId& target,
    Distance range,
    Angle threshold)
{
    Target = target;
    MaxRange = range;
    Threshold = threshold;

    if (unit == target)
    {
        return EAbilityStateResult::Complete;
    }

    if (!FeatureUnit::UnitCanTurn(world, unit))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::CannotTurn };
    }

    if (!FeatureECS::IsEntityValid(world, Target))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::TargetLost };
    }

    if (!FeatureECS::IsInRange(world, unit, Target, MaxRange))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::TargetOutOfRange };
    }

    if (FeatureECS::IsFacing(world, unit, Target, Threshold))
    {
        return EAbilityStateResult::Complete;
    }

    if (!FeatureSteering::TurnToFace(world, unit, target))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::FailedToTurn };
    }

    return EAbilityStateResult::Continue;
}

AbilityStateResult FaceEntityState::OnUpdate(WorldRef world, const UnitId& unit)
{
    if (!FeatureUnit::UnitCanTurn(world, unit))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::CannotTurn };
    }

    if (!FeatureECS::IsEntityValid(world, Target))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::TargetLost };
    }

    if (!FeatureECS::IsInRange(world, unit, Target, MaxRange))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::TargetOutOfRange };
    }

    if (FeatureECS::IsFacing(world, unit, Target, Threshold))
    {
        return EAbilityStateResult::Complete;
    }

    if (!FeatureSteering::IsTurning(world, unit))
    {
        if (FeatureSteering::HasFinishedTurning(world, unit))
        {
            return EAbilityStateResult::Complete;
        }

        return { EAbilityStateResult::Fail, AbilityStateReasons::NoLongerSeekingGoal };
    }

    return EAbilityStateResult::Continue;
}

void FaceEntityState::OnInterrupt(WorldRef world, const UnitId& unit)
{
    OnExit(world, unit);
}

void FaceEntityState::OnExit(WorldRef world, const UnitId& unit)
{
    FeatureSteering::Stop(world, unit);
}

AbilityStateResult FaceLocationState::OnEnter(
    WorldRef world,
    const UnitId& unit,
    const Vec2& target,
    Distance range,
    Angle threshold)
{
    Target = target;
    MaxRange = range;
    Threshold = threshold;

    if (!FeatureUnit::UnitCanTurn(world, unit))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::CannotTurn };
    }

    if (!FeatureECS::IsInRange(world, unit, Target, MaxRange))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::TargetOutOfRange };
    }

    if (FeatureECS::IsFacing(world, unit, Target, Threshold))
    {
        return EAbilityStateResult::Complete;
    }

    if (!FeatureSteering::TurnToFace(world, unit, target))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::FailedToTurn };
    }

    return EAbilityStateResult::Continue;
}

AbilityStateResult FaceLocationState::OnUpdate(WorldRef world, const UnitId& unit)
{
    if (!FeatureUnit::UnitCanTurn(world, unit))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::CannotTurn };
    }

    if (!FeatureECS::IsInRange(world, unit, Target, MaxRange))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::TargetOutOfRange };
    }

    if (FeatureECS::IsFacing(world, unit, Target, Threshold))
    {
        return EAbilityStateResult::Complete;
    }

    if (!FeatureSteering::IsTurning(world, unit))
    {
        if (FeatureSteering::HasFinishedTurning(world, unit))
        {
            return EAbilityStateResult::Complete;
        }

        return { EAbilityStateResult::Fail, AbilityStateReasons::NoLongerSeekingGoal };
    }

    return EAbilityStateResult::Continue;
}

void FaceLocationState::OnInterrupt(WorldRef world, const UnitId& unit)
{
    OnExit(world, unit);
}

void FaceLocationState::OnExit(WorldRef world, const UnitId& unit)
{
    FeatureSteering::Stop(world, unit);
}

AbilityStateResult FollowEntityState::OnEnter(
    WorldRef world,
    const UnitId& unit,
    const EntityId& target,
    Distance range)
{
    Target = target;
    FollowRange = range;
    return OnUpdate(world, unit);
}

AbilityStateResult FollowEntityState::OnUpdate(WorldRef world, const UnitId& unit)
{
    if (SubState == ESubState::Moving)
    {
        if (!FeatureSteering::IsSeekingGoal(world, unit))
        {
            return EAbilityStateResult::Complete;
        }

        if (world.GetSimTime() > RangeCheckTime)
        {
            RangeCheckTime = world.GetSimTime() + 1.0;
            if (FeatureECS::IsInRange(world, unit, Target, FollowRange))
            {
                return SetSubState(world, unit, ESubState::Waiting);
            }
        }
    }

    if (SubState == ESubState::Waiting)
    {
        if (world.GetSimTime() > RangeCheckTime)
        {
            RangeCheckTime = world.GetSimTime() + 1.0;
            if (!FeatureECS::IsInRange(world, unit, Target, FollowRange))
            {
                return SetSubState(world, unit, ESubState::Moving);
            }
        }
    }

    return EAbilityStateResult::Continue;
}

void FollowEntityState::OnInterrupt(WorldRef world, const UnitId& unit)
{
    OnExit(world, unit);
}

void FollowEntityState::OnExit(WorldRef world, const UnitId& unit)
{
    FeatureSteering::Stop(world, unit);
}

AbilityStateResult FollowEntityState::SetSubState(WorldRef world, const UnitId& unit, ESubState subState)
{
    if (SubState == subState)
    {
        return EAbilityStateResult::Continue;
    }

    SubState = subState;

    if (SubState == ESubState::Moving)
    {
        if (!FeatureSteering::FollowEntity(world, unit, Target))
        {
            return { EAbilityStateResult::Complete };
        }
    }
    else if (SubState == ESubState::Waiting)
    {
        FeatureSteering::Stop(world, unit);
        RangeCheckTime = world.GetSimTime() + 1.0;
    }

    return EAbilityStateResult::Continue;
}
