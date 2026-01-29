
#include "PhoenixRTS/Projectiles/ProjectilesSystem.h"

#include "PhoenixSim/Profiling.h"
#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/ECS/SystemJob.h"

#include "PhoenixSteering/FeatureSteering.h"

#include "PhoenixRTS/Projectiles/ProjectileComponent.h"
#include "PhoenixRTS/Projectiles/ProjectileId.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

namespace ProjectilesSystemDetail
{
    struct UpdateProjectilesJob
    {
        void Execute(WorldRef world, const EntityComponentSpan<TransformComponent&, ProjectileComponent&>& span) const
        {
            for (auto && [entityId, index, transformComp, projectileComp] : span)
            {
                if (projectileComp.State.ActiveState != EProjectileState::None)
                {
                    AbilityStateResult result = projectileComp.State.Update(world, ProjectileId(entityId), projectileComp);
                    if (result.Result != EAbilityStateResult::Continue)
                    {
                        FeatureECS::StaticReleaseEntity(world, entityId);
                    }
                }
            }
        }
    };
}

void ProjectilesSystem::OnWorldUpdate(WorldRef world, const SystemUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    {
        PHX_PROFILE_ZONE_SCOPED_N("UpdateProjectilesJob");
        ProjectilesSystemDetail::UpdateProjectilesJob job;
        FeatureECS::ForEachEntity(world, job);
    }
}