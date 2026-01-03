#include "PhoenixRTS/Projectiles/FeatureProjectile.h"

#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/LDS/FeatureLDS.h"
#include "PhoenixSim/Session.h"

#include "PhoenixSteering/SteeringComponent.h"

#include "PhoenixRTS/Data/DataUnit.h"
#include "PhoenixRTS/Effects/EffectComponent.h"
#include "PhoenixRTS/Effects/FeatureEffects.h"
#include "PhoenixRTS/Effects/PeriodicEffectComponent.h"
#include "PhoenixRTS/Projectiles/ProjectileComponent.h"
#include "PhoenixRTS/Projectiles/ProjectilesSystem.h"

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
        return {};

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

    // TODO (jfarris): spawn launch fx actor

    projectileComp->State.Enter(world, projectileId, *projectileComp, args.ArrivalThreshold);

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

void FeatureProjectiles::Initialize(const TSharedPtr<Phoenix::Session>& session)
{
    IFeature::Initialize(session);

    ProjectilesSystem = MakeShared<RTS::ProjectilesSystem>();

    TSharedPtr<FeatureECS> featureECS = Session->GetFeatureSet()->GetFeature<FeatureECS>();
    featureECS->RegisterSystem(ProjectilesSystem);
}

void FeatureProjectiles::Shutdown()
{
    IFeature::Shutdown();

    if (TSharedPtr<FeatureECS> featureECS = Session->GetFeatureSet()->GetFeature<FeatureECS>())
    {
        featureECS->UnregisterSystem(ProjectilesSystem);
    }

    ProjectilesSystem.reset();
}
