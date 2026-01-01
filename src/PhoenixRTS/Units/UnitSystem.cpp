
#include "PhoenixRTS/Units/UnitSystem.h"

#include "PhoenixSim/Profiling.h"
#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/ECS/SystemJob.h"

#include "PhoenixRTS/TargetFiltering/TargetScanner.h"
#include "PhoenixRTS/Units/FeatureUnit.h"
#include "PhoenixRTS/Units/UnitComponent.h"
#include "PhoenixSim/LDS/FeatureLDS.h"

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
            auto lds = LDS::FeatureLDS::StaticGetWorldQueryContext(world);

            for (auto && [entityId, index, unitComp] : span)
            {
                UnitId unit = UnitId(entityId);

                // The unit had an expiration timer set, should we release the entity?
                if (FeatureUnit::HasExpired(world, unit))
                {
                    FeatureECS::StaticReleaseEntity(world, unit);
                    continue;
                }

                if (!FeatureUnit::UnitIsDead(world, unit) && !FeatureUnit::UnitIsDormant(world, unit))
                {
                    TargetScanArgs args;
                    args.Level = FeatureUnit::GetTargetScanLevel(world, unit);
                    args.Flags = ETargetScanFlags::AutoAcquire;
                    args.LdsQueryContext = lds;
                    TargetScanner::ScanForTarget(world, unit, args);
                }
            }
        }
    };
}

void UnitSystem::OnWorldUpdate(WorldRef world, const SystemUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    UnitSystemDetail::UpdateUnitsJob job;
    FeatureECS::Schedule(world, job);
}