#include "PhoenixRTS/Effects/EffectLaunchProjectileHandler.h"

#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/LDS/FeatureLDS.h"

#include "PhoenixRTS/Data/DataEffectLaunchProjectile.h"
#include "PhoenixRTS/Effects/EffectComponent.h"
#include "PhoenixRTS/Effects/FeatureEffects.h"
#include "PhoenixRTS/Projectiles/FeatureProjectile.h"
#include "PhoenixRTS/Projectiles/ProjectileComponent.h"
#include "PhoenixSim/ECS/System.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

FName EffectLaunchProjectileHandler::GetEffectTypeId() const
{
    return "EffectLaunchProjectile"_n;
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

        Angle launchFacing = FeatureECS::GetWorldFacing(world, source);

        ProjectileId projectileId = FeatureProjectiles::SpawnProjectile(
            world,
            projectileData.GetObjectId(),
            source,
            sourcePos,
            launchFacing,
            target,
            targetPos);

        Time periodicTime = effectData.PeriodicTime().GetValue(lds);
        FName periodicEffectId = effectData.PeriodicEffect().GetReferenceId(lds);

        if (projectileId != ProjectileId::Invalid && periodicTime > 0 && !FName::IsNoneOrEmpty(periodicEffectId))
        {
            // Keep the effect alive as long as the projectile is alive
            FeatureEffects::ReferenceEffectNode(world, nodeId, *nodeComp);

            ProjectileComponent* comp = FeatureECS::GetComponent<ProjectileComponent>(world, nodeId);
            comp->EffectOwner = nodeId;
            comp->NextPeriodic = world.GetSimTime() + periodicTime;
            comp->PeriodicEffectId = periodicEffectId;
        }
    }

    return true;
}

bool EffectLaunchProjectileHandler::CanExecute(WorldConstRef world, const EffectExecuteContext& context) const
{
    return true;
}
