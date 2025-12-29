#include "PhoenixRTS/Effects/EffectLaunchProjectileHandler.h"

#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/LDS/FeatureLDS.h"

#include "PhoenixRTS/Data/DataEffectLaunchProjectile.h"
#include "PhoenixRTS/Effects/EffectComponent.h"
#include "PhoenixRTS/Effects/FeatureEffects.h"
#include "PhoenixRTS/Projectiles/FeatureProjectile.h"
#include "PhoenixSim/ECS/System.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

namespace EffectLaunchProjectileSystemDetail
{
    struct Job : IBufferJob<EffectLaunchProjectileComponent&>
    {
        void Execute(const EntityComponentSpan<EffectLaunchProjectileComponent&>& span) override
        {
            PHX_PROFILE_ZONE_SCOPED_N("EffectLaunchProjectileJob");

            WorldRef world = *World;
            auto lds = LDS::FeatureLDS::StaticGetWorldQueryContext(world);

            for (auto && [entityId, index, launchProjComp] : span)
            {
                
            }
        }
    };
}

void EffectLaunchProjectileSystem::OnWorldUpdate(WorldRef world, const ECS::SystemUpdateArgs& args)
{
    ISystem::OnWorldUpdate(world, args);

    PHX_PROFILE_ZONE_SCOPED;

    EffectLaunchProjectileSystemDetail::Job job;
    FeatureECS::ScheduleParallel(world, job);
}

FName EffectLaunchProjectileHandler::GetEffectTypeId() const
{
    return "EffectLaunchProjectile"_n;
}

void EffectLaunchProjectileHandler::Initialize(const TSharedPtr<Phoenix::Session>& session)
{
    IEffectHandler::Initialize(session);

    System = MakeShared<EffectLaunchProjectileSystem>();

    TSharedPtr<FeatureECS> featureECS = Session->GetFeatureSet()->GetFeature<FeatureECS>();
    featureECS->RegisterSystem(System);
}

void EffectLaunchProjectileHandler::Shutdown()
{
    IEffectHandler::Shutdown();

    if (TSharedPtr<FeatureECS> featureECS = Session->GetFeatureSet()->GetFeature<FeatureECS>())
    {
        featureECS->UnregisterSystem(System);
    }

    System.reset();
}

bool EffectLaunchProjectileHandler::Execute(WorldRef world, const EffectExecuteContext& context) const
{
    const LDS::ILDSQueryContext& lds = *context.LdsQueryContext;

    ExecuteEffectArgs args;
    args.EffectId = context.EffectId;
    EffectNodeId nodeId = FeatureEffects::AcquireEffectNode(world, context.ParentId, *context.ParentComponent, args);

    EffectComponent* nodeComp = FeatureEffects::GetEffectComponent(world, nodeId);
    if (!nodeComp)
    {
        return false;
    }

    Data::EffectLaunchProjectilePtr effectData(context.EffectId);

    EntityId source = nodeComp->SourceId;
    const Vec2& sourcePos = nodeComp->SourcePos;
    EntityId target = nodeComp->TargetId;
    const Vec2& targetPos = nodeComp->TargetPos;

    // TODO (jfarris): validate the effect
    // if (FeatureEffects::ValidateEffectNode())
    {
        Data::ProjectilePtr projectileData = effectData.Projectile().ResolveObject(lds);

        Vec2 launchPos = sourcePos;
        Angle launchFacing = FeatureECS::GetWorldFacing(world, source);

        ProjectileId projectileId = FeatureProjectiles::SpawnProjectile(
            world,
            projectileData.GetObjectId(),
            source,
            sourcePos,
            launchFacing,
            target,
            targetPos);

        if (projectileId != ProjectileId::Invalid)
        {
            // Keep the effect alive as long as the projectile is alive
            FeatureEffects::ReferenceEffectNode(world, nodeId, *nodeComp);

            EffectLaunchProjectileComponent* comp = FeatureECS::GetOrAddComponent<EffectLaunchProjectileComponent>(world, nodeId);
            comp->LaunchPos = launchPos;
            comp->NextPeriodic = world.GetSimTime() + effectData.PeriodicTime().GetValue(lds);
            comp->PeriodicEffectId = effectData.PeriodicEffect().ResolveObject(lds).GetObjectId();
        }
    }

    return true;
}

bool EffectLaunchProjectileHandler::CanExecute(WorldConstRef world, const EffectExecuteContext& context) const
{
    return true;
}
