
#include "PhoenixRTS/Projectiles/ProjectilesSystem.h"

#include "ProjectileComponent.h"
#include "PhoenixSim/Profiling.h"
#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/ECS/SystemJob.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

namespace ProjectilesSystemDetail
{
    struct UpdateProjectilesJob : IBufferJob<TransformComponent&, ProjectileComponent&>
    {
        void Execute(const EntityComponentSpan<TransformComponent&, ProjectileComponent&>& span) override
        {
            PHX_PROFILE_ZONE_SCOPED_N("UpdateProjectilesJob");

            for (auto && [entityId, index, transformComp, projectileComp] : span)
            {
                
            }
        }
    };
}

void ProjectilesSystem::OnWorldUpdate(WorldRef world, const SystemUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    ProjectilesSystemDetail::UpdateProjectilesJob job;
    FeatureECS::ScheduleParallel(world, job);
}