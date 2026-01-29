#include "PhoenixRTS/Projectiles/ProjectileComponent.h"

#include "PhoenixRTS/Effects/FeatureEffects.h"
#include "PhoenixRTS/Projectiles/ProjectileId.h"

using namespace Phoenix;
using namespace Phoenix::RTS;

ProjectileState::ProjectileState()
    : States(MoveToEntityState{})
    , ActiveState(EProjectileState::None)
{
}

AbilityStateResult ProjectileState::Enter(
    WorldRef world,
    const ProjectileId& projectile,
    ProjectileComponent& comp,
    Distance arrivalThreshold)
{
    ArrivalThreshold = arrivalThreshold;

    AbilityStateResult result;

    if (comp.TargetEntity != ECS::EntityId::Invalid)
    {
        ActiveState = EProjectileState::MoveToEntity;
    }
    else
    {
        ActiveState = EProjectileState::MoveToLocation;
    }

    switch (ActiveState)
    {
        case EProjectileState::MoveToEntity:    result = States.MoveToEntity.Enter(world, projectile, comp.TargetEntity, ArrivalThreshold); break;
        case EProjectileState::MoveToLocation:  result = States.MoveToLocation.Enter(world, projectile, comp.TargetPos, ArrivalThreshold); break;
        case EProjectileState::None:            return result;
    }

    return HandleActiveStateResult(world, projectile, comp, result);
}

AbilityStateResult ProjectileState::Update(WorldRef world, const ProjectileId& projectile, ProjectileComponent& comp)
{
    AbilityStateResult result;

    switch (ActiveState)
    {
        case EProjectileState::MoveToEntity:    result = States.MoveToEntity.Update(world, projectile); break;
        case EProjectileState::MoveToLocation:  result = States.MoveToLocation.Update(world, projectile); break;
        case EProjectileState::None:            return result;
    }

    return HandleActiveStateResult(world, projectile, comp, result);
}

void ProjectileState::Interrupt(WorldRef world, const ProjectileId& projectile, const ProjectileComponent& comp)
{
    switch (ActiveState)
    {
        case EProjectileState::MoveToEntity:    States.MoveToEntity.Interrupt(world, projectile); break;
        case EProjectileState::MoveToLocation:  States.MoveToLocation.Interrupt(world, projectile); break;
        case EProjectileState::None:            break;
    }

    Exit(world, projectile, comp);
}

void ProjectileState::Exit(WorldRef world, const ProjectileId& projectile, const ProjectileComponent& comp)
{
    switch (ActiveState)
    {
        case EProjectileState::MoveToEntity:    States.MoveToEntity.Exit(world, projectile); break;
        case EProjectileState::MoveToLocation:  States.MoveToLocation.Exit(world, projectile); break;
        case EProjectileState::None:            break;
    }

    FeatureEffects::DereferenceEffectNode(world, comp.EffectParent);
}

AbilityStateResult ProjectileState::HandleActiveStateResult(
    WorldRef world,
    const ProjectileId& projectile,
    ProjectileComponent& comp,
    const AbilityStateResult& result) const
{
    if (result.Result == EAbilityStateResult::Complete)
    {
        FeatureEffects::StaticExecuteEffect(world, comp.EffectParent, comp.ImpactEffectId);
    }

    // if (result.Result == EAbilityStateResult::Fail)
    // {
    //     if ((result.Reason == AbilityStateReasons::TargetLost || result.Reason == AbilityStateReasons::TargetInvalid) &&
    //         !Vec2::Equals(comp.State.States.MoveToEntity.LastKnownPosition, Vec2::Max))
    //     {
    //         comp.State.ActiveState = EProjectileState::MoveToLocation;
    //         return comp.State.States.MoveToLocation.Enter(world, projectile, comp.State.States.MoveToEntity.LastKnownPosition, ArrivalThreshold);
    //     }
    // }

    return result;
}
