#include "Phoenix.Sim.RTS/Projectiles/FeatureProjectile.h"

#include "ProjectileTask.h"
#include "Phoenix.Sim.ECS/FeatureECS.h"
#include "Phoenix.Sim.LDS/FeatureLDS.h"
#include "Phoenix.Sim/Session.h"

#include "Phoenix.Sim.Steering/SteeringComponent.h"

#include "Phoenix.Sim.RTS/Data/DataUnit.h"
#include "Phoenix.Sim.RTS/Effects/EffectComponent.h"
#include "Phoenix.Sim.RTS/Effects/FeatureEffects.h"
#include "Phoenix.Sim.RTS/Effects/PeriodicEffectComponent.h"
#include "Phoenix.Sim.RTS/Projectiles/ProjectileComponent.h"
#include "Phoenix.Sim.Tasks/FeatureTask.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::ECS;
using namespace Phoenix::Steering;
using namespace Phoenix::RTS;

ProjectileId FeatureProjectiles::SpawnProjectile(
    WorldRef world,
    const FName& projectileData,
    const Vec2& launchPos,
    Angle launchFacing,
    const SpawnProjectileArgs& args)
{
    ProjectileId projectileId = ProjectileId(FeatureECS::StaticAcquireEntity(world, "Projectile"_n));
    if (projectileId == EntityId::Invalid)
    {
        return {};
    }

    const ILDSQueryContext& lds = *FeatureLDS::StaticGetWorldQueryContext(world);

    Data::ProjectilePtr dataPtr(projectileData);

    TransformComponent* transformComp = FeatureECS::GetOrAddComponent<TransformComponent>(world, projectileId);
    transformComp->Transform.Position = launchPos;
    transformComp->Transform.Rotation = launchFacing;

    SteeringComponent* steeringComp = FeatureECS::GetOrAddComponent<SteeringComponent>(world, projectileId);
    steeringComp->AccelerationTime = dataPtr.Movement().AccelerationTime().GetValue(lds);
    steeringComp->MaxSpeed = dataPtr.Movement().MaxSpeed().GetValue(lds);
    steeringComp->OuterRadius = dataPtr.Movement().Radius().GetValue(lds);
    steeringComp->CollisionMask = static_cast<uint32>(Data::ECollisionFlags::None);

    Data::EProjectileMovementFlags projMoveFlags = dataPtr.Movement().Flags().GetValue(lds);
    if (HasAnyFlags(projMoveFlags, Data::EProjectileMovementFlags::LockFacing))
    {
        SetFlagRef(steeringComp->Flags, ESteerFlags::LockFacing);
    }

    ProjectileComponent* projectileComp = FeatureECS::GetOrAddComponent<ProjectileComponent>(world, projectileId);
    projectileComp->Owner = args.Owner;
    projectileComp->ProjectileDataId = projectileData;
    projectileComp->LaunchPos = launchPos;
    projectileComp->TargetEntity = args.TargetEntity;
    projectileComp->TargetPos = args.TargetPos;
    projectileComp->ImpactEffectId = args.ImpactEffectId;

    // Keep the parent effect alive as long as the projectile is alive
    if (EffectComponent* effectParentComp = FeatureEffects::GetEffectComponent(world, args.EffectParent))
    {
        projectileComp->EffectParent = args.EffectParent;

        FeatureEffects::ReferenceEffectNode(world, args.EffectParent, *effectParentComp);

        if (args.PeriodicEffectTime > 0 && !FName::IsNoneOrEmpty(args.PeriodicEffectId))
        {
            PeriodicEffectComponent* periodicEffectComponent = FeatureECS::GetOrAddComponent<PeriodicEffectComponent>(world, projectileId);
            periodicEffectComponent->EffectNode = args.EffectParent;
            periodicEffectComponent->PeriodicEffectTime = args.PeriodicEffectTime;
            periodicEffectComponent->NextPeriodicEffectTime = world.GetSimTime() + args.PeriodicEffectTime;
            periodicEffectComponent->PeriodicEffectId = args.PeriodicEffectId;
        }

        if (!FName::IsNoneOrEmpty(args.LaunchEffectId))
        {
            FeatureEffects::StaticExecuteEffect(world, args.EffectParent, args.LaunchEffectId);
        }
    }

    // Allocate task to execute projectile logic
    {
        ProjectileTask task;
        task.State.TargetEntity = projectileComp->TargetEntity;
        task.State.TargetPos = projectileComp->TargetPos;
        task.State.EffectParent = projectileComp->EffectParent;
        task.State.ImpactEffectId = projectileComp->ImpactEffectId;
        task.State.ArrivalThreshold = args.ArrivalThreshold;
        Tasks::FeatureTask::Allocate(world, projectileId, task);
    }

    // TODO (jfarris): spawn launch fx actor

    return projectileId;
}

EntityId FeatureProjectiles::GetOwner(WorldConstRef world, ProjectileId projectileId)
{
    const ProjectileComponent* comp = FeatureECS::GetComponent<ProjectileComponent>(world, projectileId);
    return comp ? comp->Owner : EntityId::Invalid;
}

FName FeatureProjectiles::GetDataId(WorldConstRef world, ProjectileId projectileId)
{
    const ProjectileComponent* comp = FeatureECS::GetComponent<ProjectileComponent>(world, projectileId);
    return comp ? comp->ProjectileDataId : FName::None;
}

RTS::Data::ProjectilePtr FeatureProjectiles::GetData(WorldConstRef world, ProjectileId projectileId)
{
    return { GetDataId(world, projectileId) };
}
