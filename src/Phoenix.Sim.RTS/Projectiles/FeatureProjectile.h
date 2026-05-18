#pragma once

#include "Phoenix.Sim/Features.h"

#include "Phoenix.Sim.RTS/DLLExport.h"
#include "Phoenix.Sim.RTS/Data/DataProjectile.h"
#include "Phoenix.Sim.RTS/Effects/EffectId.h"
#include "Phoenix.Sim.RTS/Projectiles/ProjectileId.h"

namespace Phoenix::RTS
{
    class ProjectilesSystem;

    struct PHOENIX_RTS_API SpawnProjectileArgs
    {
        ECS::EntityId Owner;
        ECS::EntityId TargetEntity;
        Vec2 TargetPos;
        EffectNodeId EffectParent;
        FName ImpactEffectId;
        FName LaunchEffectId;
        Time PeriodicEffectTime;
        FName PeriodicEffectId;
        Distance ArrivalThreshold = 0.1;
    };

    class PHOENIX_RTS_API FeatureProjectiles : public IFeature
    {
        PHX_DECLARE_FEATURE_TYPE(FeatureProjectiles){}

    public:

        static ProjectileId SpawnProjectile(
            WorldRef world,
            const FName& projectileData,
            const Vec2& launchPos,
            Angle launchFacing,
            const SpawnProjectileArgs& args = {});

        static ECS::EntityId GetOwner(WorldConstRef world, ProjectileId projectileId);

        static FName GetDataId(WorldConstRef world, ProjectileId projectileId);
        static Data::ProjectilePtr GetData(WorldConstRef world, ProjectileId projectileId);
    };
}
