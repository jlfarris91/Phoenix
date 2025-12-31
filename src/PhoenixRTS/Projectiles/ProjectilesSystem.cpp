
#include "PhoenixRTS/Projectiles/ProjectilesSystem.h"

#include "PhoenixSim/Profiling.h"
#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/ECS/SystemJob.h"
#include "PhoenixSim/LDS/FeatureLDS.h"

#include "PhoenixSteering/FeatureSteering.h"

#include "PhoenixRTS/Data/DataProjectile.h"
#include "PhoenixRTS/Projectiles/ProjectileComponent.h"

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

            WorldRef world = *World;
            const LDS::ILDSQueryContext& lds = *LDS::FeatureLDS::StaticGetWorldQueryContext(world);
            
            for (auto && [entityId, index, transformComp, projectileComp] : span)
            {
                Distance range = RTS::Data::ProjectilePtr(projectileComp.ProjectileDataId).Movement().Radius().GetValue(lds);
                if (!Steering::FeatureSteering::IsSeekingGoal(world, entityId) ||
                    FeatureECS::IsInRange(world, entityId, projectileComp.TargetEntity, range))
                {
                    FeatureECS::ReleaseEntity(world, entityId);
                }
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