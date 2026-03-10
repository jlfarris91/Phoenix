#pragma once

#include "ProjectileId.h"
#include "PhoenixSim/Features.h"

#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Data/DataProjectile.h"
#include "PhoenixRTS/Effects/EffectId.h"

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
        PHX_DECLARE_FEATURE_TYPE(FeatureProjectiles)

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

    protected:

        void Initialize(const std::shared_ptr<Phoenix::Session>& session) override;
        void Shutdown() override;

        std::shared_ptr<ProjectilesSystem> ProjectilesSystem;
    };
}
