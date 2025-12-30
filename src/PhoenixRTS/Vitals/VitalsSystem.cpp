
#include "PhoenixRTS/Vitals/VitalsSystem.h"

#include "PhoenixSim/Profiling.h"
#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/ECS/SystemJob.h"

#include "PhoenixRTS/Units/UnitId.h"
#include "PhoenixRTS/Vitals/Damage.h"
#include "PhoenixRTS/Vitals/FeatureVitals.h"
#include "PhoenixRTS/Vitals/VitalComponents.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

namespace VitalsSystemDetail
{
    struct UpdateHealthComponentJob : IBufferJob<HealthComponent&>
    {
        void Execute(const EntityComponentSpan<HealthComponent&>& span) override
        {
            PHX_PROFILE_ZONE_SCOPED_N("UpdateVitalsJob");

            for (auto && [entityId, index, healthComp] : span)
            {
                if (healthComp.Health.Regen < 0.0)
                {
                    Damage damage;
                    damage.VitalId = "HealthVital"_n;
                    damage.SourceId = entityId;
                    damage.Amount = -healthComp.Health.Regen;
                    damage.BaseAmount = damage.Amount;
                    damage.ArmorMultiplier = 0;
                    FeatureVitals::ApplyDamage(*World, UnitId(entityId), damage);
                }
                else
                {
                    healthComp.Health.Current += healthComp.Health.Regen;
                    if (healthComp.Health.Current >= healthComp.Health.Max)
                    {
                        healthComp.Health.Current = healthComp.Health.Max;
                    }
                }
            }
        }
    };
}

void VitalsSystem::OnWorldUpdate(WorldRef world, const SystemUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    VitalsSystemDetail::UpdateHealthComponentJob job;
    FeatureECS::Schedule(world, job);
}