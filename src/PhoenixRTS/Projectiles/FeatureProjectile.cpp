#include "PhoenixRTS/Projectiles/FeatureProjectile.h"

#include "PhoenixRTS/Data/DataUnit.h"
#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/LDS/FeatureLDS.h"

#include "PhoenixSteering/SteeringComponent.h"

#include "PhoenixRTS/Projectiles/ProjectileComponent.h"
#include "PhoenixRTS/Projectiles/ProjectilesSystem.h"
#include "PhoenixSteering/FeatureSteering.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::ECS;
using namespace Phoenix::Steering;
using namespace Phoenix::RTS;

FeatureProjectiles::FeatureProjectiles()
{
}

ProjectileId FeatureProjectiles::SpawnProjectile(
    WorldRef world,
    const FName& projectileData,
    EntityId owner,
    const Vec2& launchPos,
    Angle launchFacing,
    EntityId target,
    const Vec2& targetPos)
{
    ProjectileId projectileId = ProjectileId(FeatureECS::AcquireEntity(world, "Projectile"_n));
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
        SetFlagRef(steeringComp->Flags, ESteeringFlags::LockFacing);
    }

    ProjectileComponent* projectileComp = FeatureECS::GetOrAddComponent<ProjectileComponent>(world, projectileId);
    projectileComp->Owner = owner;
    projectileComp->ProjectileDataId = projectileData;
    projectileComp->LaunchPos = launchPos;
    projectileComp->TargetEntity = target;
    projectileComp->TargetPos = targetPos;

    if (FeatureECS::IsEntityValid(world, target))
    {
        FeatureSteering::FollowEntity(world, projectileId, target);
    }
    else
    {
        FeatureSteering::MoveToLocation(world, projectileId, targetPos);
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
