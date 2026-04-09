#include "PhoenixRTS/Units/UnitSystem.h"

#include "PhoenixSim/ECS/ECSCommands.h"
#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/ECS/SystemJob.h"
#include "PhoenixSim/LDS/FeatureLDS.h"

#include "PhoenixRTS/ECS/ECSCommands.h"
#include "PhoenixRTS/Orders/FeatureOrders.h"
#include "PhoenixRTS/TargetFiltering/TargetScanner.h"
#include "PhoenixRTS/Units/FeatureUnit.h"
#include "PhoenixRTS/Units/UnitComponent.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

namespace UnitSystemDetail
{
    class UpdateUnitsJob : public IJob<const UnitComponent&>
    {
    public:

        virtual FName GetName() const override
        {
            return "UpdateUnitsJob"_n;
        }

        void BeginBatch(WorldConstRef world, const JobBatch& batch, CommandBuffer& cb) override
        {
            LDSQueryContext = FeatureLDS::StaticGetWorldQueryContext(world);
        }

        void Execute(WorldConstRef world, EntityId id, CommandBuffer& cb, const UnitComponent&) override
        {
            UnitId unit = UnitId(id);

            // The unit had an expiration timer set, should we release the entity?
            if (FeatureUnit::HasExpired(world, unit))
            {
                cb.Append<Commands::ReleaseEntity>(unit);
                return;
            }

            if (!FeatureUnit::UnitIsDead(world, unit) && !FeatureUnit::UnitIsDormant(world, unit))
            {
                Time simTime = world.GetSimTime();
                Time nextScanTime = FeatureECS::GetBlackboardValue<Time>(world, unit, "NextTargetScanTime"_n);

                if (simTime > nextScanTime)
                {
                    TargetScanArgs args;
                    args.Level = FeatureUnit::GetTargetScanLevel(world, unit);
                    args.LdsQueryContext = LDSQueryContext;
                    auto scanResult = TargetScanner::ScanForTarget(world, unit, args);
                    if (scanResult.AcquireRequest.IsSet())
                    {
                        cb.Append<Commands::RequestAcquireOrder>(unit, scanResult.AcquireRequest.Get());
                    }

                    nextScanTime = simTime + 0.25;
                    cb.Append<Commands::SetBlackboardValue<Time>>(id, "NextTargetScanTime"_n, nextScanTime);
                }
            }
        }

        void EndBatch(WorldConstRef, const JobBatch& batch, CommandBuffer& cb) override
        {
            LDSQueryContext.reset();
        }

    private:

        std::shared_ptr<const ILDSQueryContext> LDSQueryContext;
    };
}

void UnitSystem::OnWorldInitialize(WorldRef world)
{
    ISystem::OnWorldInitialize(world);

    FeatureECS::RegisterJob(world, std::make_unique<UnitSystemDetail::UpdateUnitsJob>());

    FeatureECS::RegisterCommandHandler<Commands::RequestAcquireOrder>(world, [](WorldRef world, const Commands::RequestAcquireOrder& command)
    {
        FeatureOrders::StaticRequestAcquireOrder(world, command.Target, command.Request);
    });
}
