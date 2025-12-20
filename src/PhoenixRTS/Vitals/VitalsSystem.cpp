
#include "PhoenixRTS/Vitals/VitalsSystem.h"

#include "PhoenixSim/Profiling.h"
#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/ECS/SystemJob.h"

#include "PhoenixRTS/Vitals/VitalsComponent.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

namespace VitalsSystemDetail
{
    struct UpdateVitalsJob : IBufferJob<VitalsComponent&>
    {
        void Execute(const EntityComponentSpan<VitalsComponent&>& span) override
        {
            PHX_PROFILE_ZONE_SCOPED_N("UpdateVitalsJob");

            for (auto && [entityId, index, vitalsComp] : span)
            {
                vitalsComp.Health.Current += vitalsComp.Health.Regen;
                if (vitalsComp.Health.Current >= vitalsComp.Health.Max)
                {
                    vitalsComp.Health.Current = vitalsComp.Health.Max;
                }

                vitalsComp.Energy.Current += vitalsComp.Energy.Regen;
                if (vitalsComp.Energy.Current >= vitalsComp.Energy.Max)
                {
                    vitalsComp.Energy.Current = vitalsComp.Energy.Max;
                }

                vitalsComp.Shield.Current += vitalsComp.Shield.Regen;
                if (vitalsComp.Shield.Current >= vitalsComp.Shield.Max)
                {
                    vitalsComp.Shield.Current = vitalsComp.Shield.Max;
                }
            }
        }
    };
}

void VitalsSystem::OnWorldUpdate(WorldRef world, const SystemUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    VitalsSystemDetail::UpdateVitalsJob job;
    FeatureECS::ScheduleParallel(world, job);
}