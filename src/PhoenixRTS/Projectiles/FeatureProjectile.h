#pragma once

#include "ProjectileId.h"
#include "PhoenixSim/Features.h"
#include "PhoenixSim/Session.h"

#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Data/DataProjectile.h"

namespace Phoenix::RTS
{
    class ProjectilesSystem;

    class PHOENIX_RTS_API FeatureProjectiles : public IFeature
    {
        PHX_DECLARE_FEATURE_TYPE(FeatureProjectiles)

    public:

        FeatureProjectiles();

        static ProjectileId SpawnProjectile(
            WorldRef world,
            const FName& projectileData,
            ECS::EntityId owner,
            const Vec2& launchPos,
            Angle launchFacing,
            ECS::EntityId target,
            const Vec2& targetPos);

        static ECS::EntityId GetOwner(WorldConstRef world, ProjectileId projectileId);

        static FName GetDataId(WorldConstRef world, ProjectileId projectileId);
        static Data::ProjectilePtr GetData(WorldConstRef world, ProjectileId projectileId);

    protected:

        void Initialize(const TSharedPtr<Phoenix::Session>& session) override;
        void Shutdown() override;

        TSharedPtr<ProjectilesSystem> ProjectilesSystem;
    };
}
