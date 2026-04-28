#include "ProjectileState.h"

#include "PhoenixRTS/Effects/FeatureEffects.h"
#include "PhoenixRTS/Projectiles/ProjectileId.h"

using namespace Phoenix;
using namespace Phoenix::RTS;

ProjectileState::ProjectileState()
    : States(MoveToEntityState{})
    , ActiveState(EProjectileState::Idle)
{
}

AbilityStateResult ProjectileState::Enter(WorldRef world, const ProjectileId& projectile)
{
    AbilityStateResult result;

    if (TargetEntity != ECS::EntityId::Invalid)
    {
        ActiveState = EProjectileState::MoveToEntity;
    }
    else
    {
        ActiveState = EProjectileState::MoveToLocation;
    }

    switch (ActiveState)
    {
        case EProjectileState::MoveToEntity:    result = States.MoveToEntity.Enter(world, projectile, TargetEntity, ArrivalThreshold); break;
        case EProjectileState::MoveToLocation:  result = States.MoveToLocation.Enter(world, projectile, TargetPos, ArrivalThreshold); break;
        case EProjectileState::Idle:            return result;
    }

    return HandleActiveStateResult(world, projectile, result);
}

AbilityStateResult ProjectileState::Update(WorldRef world, const ProjectileId& projectile)
{
    AbilityStateResult result;

    switch (ActiveState)
    {
        case EProjectileState::MoveToEntity:    result = States.MoveToEntity.Update(world, projectile); break;
        case EProjectileState::MoveToLocation:  result = States.MoveToLocation.Update(world, projectile); break;
        case EProjectileState::Idle:            return result;
    }

    return HandleActiveStateResult(world, projectile, result);
}

void ProjectileState::Interrupt(WorldRef world, const ProjectileId& projectile)
{
    switch (ActiveState)
    {
        case EProjectileState::MoveToEntity:    States.MoveToEntity.Interrupt(world, projectile); break;
        case EProjectileState::MoveToLocation:  States.MoveToLocation.Interrupt(world, projectile); break;
        case EProjectileState::Idle:            break;
    }

    Exit(world, projectile);
}

void ProjectileState::Exit(WorldRef world, const ProjectileId& projectile)
{
    switch (ActiveState)
    {
        case EProjectileState::MoveToEntity:    States.MoveToEntity.Exit(world, projectile); break;
        case EProjectileState::MoveToLocation:  States.MoveToLocation.Exit(world, projectile); break;
        case EProjectileState::Idle:            break;
    }

    FeatureEffects::DereferenceEffectNode(world, EffectParent);
}

AbilityStateResult ProjectileState::HandleActiveStateResult(
    WorldRef world,
    const ProjectileId& projectile,
    const AbilityStateResult& result) const
{
    if (result.Result == EAbilityStateResult::Complete)
    {
        FeatureEffects::StaticExecuteEffect(world, EffectParent, ImpactEffectId);
    }

    // if (result.Result == EAbilityStateResult::Fail)
    // {
    //     if ((result.Reason == AbilityStateReasons::TargetLost || result.Reason == AbilityStateReasons::TargetInvalid) &&
    //         !Vec2::Equals(States.MoveToEntity.LastKnownPosition, Vec2::Max))
    //     {
    //         ActiveState = EProjectileState::MoveToLocation;
    //         return States.MoveToLocation.Enter(world, projectile, States.MoveToEntity.LastKnownPosition, ArrivalThreshold);
    //     }
    // }

    return result;
}
