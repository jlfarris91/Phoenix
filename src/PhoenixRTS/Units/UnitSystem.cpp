
#include "PhoenixRTS/Units/UnitSystem.h"

#include "FeatureUnit.h"
#include "PhoenixSim/Profiling.h"
#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/ECS/SystemJob.h"

#include "PhoenixRTS/Units/UnitComponent.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

namespace UnitSystemDetail
{
    struct UpdateUnitsJob : IBufferJob<UnitComponent&>
    {
        void Execute(const EntityComponentSpan<UnitComponent&>& span) override
        {
            PHX_PROFILE_ZONE_SCOPED_N("UpdateUnitsJob");

            WorldRef world = *World;

            for (auto && [entityId, index, unitComp] : span)
            {
                UnitId unit = UnitId(entityId);

                // The unit had an expiration timer set, should we release the entity?
                if (FeatureUnit::HasExpired(world, unit))
                {
                    FeatureECS::ReleaseEntity(world, unit);
                }
            }
        }
    };
}

void UnitSystem::OnWorldUpdate(WorldRef world, const SystemUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    UnitSystemDetail::UpdateUnitsJob job;
    FeatureECS::ScheduleParallel(world, job);
}