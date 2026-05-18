#include "ProjectileTask.h"

#include "ProjectileComponent.h"
#include "ProjectileId.h"
#include "ProjectileState.h"

void Phoenix::RTS::ProjectileTask::OnCreate(WorldRef world, uint32 context)
{
    SetIntervalTicks(world, 3);

    State.Enter(world, ProjectileId(context));
}

void Phoenix::RTS::ProjectileTask::OnUpdate(WorldRef world, uint32 context)
{
    ProjectileId projectile = ProjectileId(context);

    if (State.ActiveState == EProjectileState::Idle)
    {
        return;
    }

    AbilityStateResult result = State.Update(world, projectile);

    if (result != EAbilityStateResult::Continue)
    {
        State.Exit(world, projectile);
        ECS::FeatureECS::StaticReleaseEntity(world, projectile);
    }
}

void Phoenix::RTS::ProjectileTask::OnFinish(WorldRef world, uint32 context)
{
    ProjectileId projectile = ProjectileId(context);

    State.Exit(world, projectile);
    ECS::FeatureECS::StaticReleaseEntity(world, projectile);
}
