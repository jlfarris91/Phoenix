#include "PhoenixRTS/Units/UnitSystem.h"

#include "PhoenixSim/Profiling.h"
#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/ECS/SystemJob.h"

#include "PhoenixRTS/TargetFiltering/TargetScanner.h"
#include "PhoenixRTS/Units/FeatureUnit.h"
#include "PhoenixRTS/Units/UnitComponent.h"
#include "PhoenixSim/LDS/FeatureLDS.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

namespace UnitSystemDetail
{
    struct UpdateUnitsJob
    {
        std::shared_ptr<const ILDSQueryContext> LDSQueryContext;

        void Begin(WorldRef world)
        {
            LDSQueryContext = FeatureLDS::StaticGetWorldQueryContext(world);
        }

        void Execute(WorldRef world, const EntityComponentSpan<UnitComponent&>& span) const
        {
            Time simTime = world.GetSimTime();
            for (auto && [entityId, index, unitComp] : span)
            {
                UnitId unitId = UnitId(entityId);

                // The unit had an expiration timer set, should we release the entity?
                if (FeatureUnit::HasExpired(world, unitId))
                {
                    FeatureECS::StaticReleaseEntity(world, unitId);
                    continue;
                }

                if (!FeatureUnit::UnitIsDead(world, unitId) && !FeatureUnit::UnitIsDormant(world, unitId))
                {
                    Time nextScanTime = FeatureECS::GetBlackboardValue<Time>(world, unitId, "NextTargetScanTime"_n);
                    if (simTime > nextScanTime)
                    {
                        TargetScanArgs args;
                        args.Level = FeatureUnit::GetTargetScanLevel(world, unitId);
                        args.Flags = ETargetScanFlags::AutoAcquire;
                        args.LdsQueryContext = LDSQueryContext;
                        TargetScanner::ScanForTarget(world, unitId, args);

                        nextScanTime = simTime + 0.25;
                        FeatureECS::SetBlackboardValue(world, unitId, "NextTargetScanTime"_n, nextScanTime);
                    }
                }
            }
        }

        void End(WorldRef)
        {
            LDSQueryContext.reset();
        }
    };
}

void UnitSystem::OnWorldUpdate(WorldRef world, const SystemUpdateArgs& args)
{
    ISystem::OnWorldUpdate(world, args);

    {
        PHX_PROFILE_ZONE_SCOPED_N("UpdateUnitsJob");
        UnitSystemDetail::UpdateUnitsJob job;
        FeatureECS::ForEachEntity(world, job);
    }
}