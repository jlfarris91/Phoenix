#include "Phoenix.Sim.RTS/Units/UnitSystem.h"

#include "Phoenix.Sim/ECS/ECSCommands.h"
#include "Phoenix.Sim/ECS/FeatureECS.h"
#include "Phoenix.Sim/ECS/SystemJob.h"
#include "Phoenix.Sim/LDS/FeatureLDS.h"

#include "Phoenix.Sim.RTS/ECS/ECSCommands.h"
#include "Phoenix.Sim.RTS/Orders/FeatureOrders.h"
#include "Phoenix.Sim.RTS/TargetFiltering/TargetScanner.h"
#include "Phoenix.Sim.RTS/Units/FeatureUnit.h"
#include "Phoenix.Sim.RTS/Units/UnitComponent.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

namespace UnitSystemDetail
{
    class UpdateUnitsJob : public IJob<const UnitComponent&>
    {
    public:

        virtual const char* GetName() const override { return "Units.UpdateUnitsJob"; }

        void BeginBatch(WorldConstRef world, const JobBatch& batch, CommandBuffer& cb) override
        {
            LDSQueryContext = FeatureLDS::StaticGetWorldQueryContext(world).get();
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
            LDSQueryContext = nullptr;
        }

    private:

        thread_local static const ILDSQueryContext* LDSQueryContext;
    };

    thread_local const ILDSQueryContext* UpdateUnitsJob::LDSQueryContext = nullptr;
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
