#include "Phoenix.Sim.RTS/Effects/EffectLaunchProjectileHandler.h"

#include "Phoenix.Sim/ECS/FeatureECS.h"
#include "Phoenix.Sim/LDS/FeatureLDS.h"

#include "Phoenix.Sim.RTS/Data/DataEffectLaunchProjectile.h"
#include "Phoenix.Sim.RTS/Effects/EffectComponent.h"
#include "Phoenix.Sim.RTS/Effects/FeatureEffects.h"
#include "Phoenix.Sim.RTS/Projectiles/FeatureProjectile.h"
#include "Phoenix.Sim.RTS/Projectiles/ProjectileComponent.h"
#include "Phoenix.Sim/ECS/System.h"

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
    args.Name = GetEffectTypeId();
    args.EffectId = context.EffectId;
    EffectNodeId nodeId = FeatureEffects::AcquireEffectNode(world, context.ParentId, *context.ParentComponent, args);

    EffectComponent* nodeComp = FeatureEffects::GetEffectComponent(world, nodeId);
    if (!nodeComp)
    {
        return false;
    }

    Data::EffectLaunchProjectilePtr effectData(context.EffectId);

    EntityId source = nodeComp->SourceId;
    EntityId target = nodeComp->TargetId;
    const Vec2& targetPos = nodeComp->TargetPos;

    // TODO (jfarris): validate the effect
    // if (FeatureEffects::ValidateEffectNode())
    {
        Data::ProjectilePtr projectileData = effectData.Projectile().ResolveObject(lds);

        const Vec2& launchPos = nodeComp->SourcePos;
        Angle launchFacing = (targetPos - launchPos).AsRadians();

        SpawnProjectileArgs spawnArgs;
        spawnArgs.Owner = source;
        spawnArgs.TargetEntity = target;
        spawnArgs.TargetPos = targetPos;
        spawnArgs.EffectParent = nodeId;
        spawnArgs.ImpactEffectId = effectData.ImpactEffect().GetReferenceId(lds);
        spawnArgs.LaunchEffectId = effectData.LaunchEffect().GetReferenceId(lds);
        spawnArgs.PeriodicEffectId = effectData.PeriodicEffect().GetReferenceId(lds);
        spawnArgs.PeriodicEffectTime = effectData.PeriodicTime().GetValue(lds);

        FeatureProjectiles::SpawnProjectile(
            world,
            projectileData.GetObjectId(),
            launchPos,
            launchFacing,
            spawnArgs);
    }

    // The projectile keeps the effect alive by incrementing the ref count
    FeatureEffects::DereferenceEffectNode(world, nodeId);

    return true;
}

bool EffectLaunchProjectileHandler::CanExecute(WorldConstRef world, const EffectExecuteContext& context) const
{
    return true;
}
