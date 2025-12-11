#include "Abilities/AbilityStates.h"

#include "FeatureSteering.h"
#include "Units/FeatureUnit.h"

using namespace Phoenix; 
using namespace Phoenix::ECS; 
using namespace Phoenix::Steering; 
using namespace Phoenix::RTS; 

EAbilityStateResult MoveToPositionState::OnEnter(WorldRef world, const UnitId& unit, const Vec2& target, Distance range)
{
    Target = target;
    Range = range;

    if (FeatureECS::IsInRange(world, unit, target, range))
    {
        return EAbilityStateResult::Complete;
    }

    if (!FeatureUnit::CanUnitMove(world, unit))
    {
        return EAbilityStateResult::Fail;
    }

    if (!FeatureSteering::MoveToLocation(world, unit, target))
    {
        return EAbilityStateResult::Fail;
    }

    return EAbilityStateResult::Continue;
}

EAbilityStateResult MoveToPositionState::OnUpdate(WorldRef world, const UnitId& unit)
{
    if (FeatureUnit::IsImmobilized(world, unit))
    {
        return EAbilityStateResult::Continue;
    }

    if (FeatureECS::IsInRange(world, unit, Target, Range))
    {
        return EAbilityStateResult::Complete;
    }

    if (!FeatureSteering::IsSeekingGoal(world, unit))
    {
        if (FeatureSteering::HasArrivedAtGoal(world, unit))
        {
            return EAbilityStateResult::Complete;
        }

        return EAbilityStateResult::Fail;
    }

    return EAbilityStateResult::Continue;
}

void MoveToPositionState::OnInterrupt(WorldRef world, const UnitId& unit)
{
    OnExit(world, unit);
}

void MoveToPositionState::OnExit(WorldRef world, const UnitId& unit)
{
    FeatureSteering::Stop(world, unit);
}
