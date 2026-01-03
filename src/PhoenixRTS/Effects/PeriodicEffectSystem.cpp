
#include "PhoenixRTS/Effects/PeriodicEffectSystem.h"

#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/ECS/SystemJob.h"
#include "PhoenixSim/LDS/FeatureLDS.h"
#include "PhoenixSim/Profiling.h"
#include "PhoenixSim/Session.h"

#include "PhoenixRTS/Effects/FeatureEffects.h"
#include "PhoenixRTS/Effects/PeriodicEffectComponent.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

namespace PeriodicEffectSystemDetail
{
    struct PeriodicEffectSystemJob : IBufferJob<TransformComponent&, PeriodicEffectComponent&>
    {
        void Execute(const EntityComponentSpan<TransformComponent&, PeriodicEffectComponent&>& span) override
        {
            PHX_PROFILE_ZONE_SCOPED_N("PeriodicEffectSystemJob");

            WorldRef world = *World;
            TSharedPtr<FeatureEffects> effectsFeature = GetFeature<FeatureEffects>(world);

            for (auto && [entityId, index, transformComp, projectileComp] : span)
            {
                if (projectileComp.PeriodicEffectTime <= 0.0 || FName::IsNoneOrEmpty(projectileComp.PeriodicEffectId))
                {
                    continue;
                }

                if (world.GetSimTime() < projectileComp.NextPeriodicEffectTime)
                {
                    continue;
                }

                effectsFeature->ExecuteEffect(world, projectileComp.EffectNode, projectileComp.PeriodicEffectId);

                projectileComp.NextPeriodicEffectTime = world.GetSimTime() + projectileComp.PeriodicEffectTime;
            }
        }
    };
}

void PeriodicEffectSystem::OnWorldUpdate(WorldRef world, const SystemUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    PeriodicEffectSystemDetail::PeriodicEffectSystemJob job;
    FeatureECS::ScheduleParallel(world, job);
}