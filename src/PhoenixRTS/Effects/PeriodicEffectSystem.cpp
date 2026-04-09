
#include "PhoenixRTS/Effects/PeriodicEffectSystem.h"

#include "PhoenixRTS/ECS/ECSCommands.h"
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

namespace
{
    struct PeriodicEffectSystemJob : IJob<PeriodicEffectComponent&>
    {
        FName GetName() const override
        {
            return "PeriodicEffectSystemJob"_n;
        }

        void Execute(
            WorldConstRef world,
            EntityId entityId,
            CommandBuffer& cb,
            PeriodicEffectComponent& periodicComp) override
        {
            if (periodicComp.PeriodicEffectTime <= 0.0 || FName::IsNoneOrEmpty(periodicComp.PeriodicEffectId))
            {
                return;
            }

            if (world.GetSimTime() < periodicComp.NextPeriodicEffectTime)
            {
                return;
            }

            periodicComp.NextPeriodicEffectTime = world.GetSimTime() + periodicComp.PeriodicEffectTime;

            cb.Append(Commands::ExecuteEffectCommand{ periodicComp.EffectNode, periodicComp.PeriodicEffectId });
        }
    };
}

void PeriodicEffectSystem::OnWorldInitialize(WorldRef world)
{
    auto job = std::make_unique<PeriodicEffectSystemJob>();
    FeatureECS::RegisterJob(world, std::move(job), EJobPhase::Update);

    FeatureECS::RegisterCommandHandler<Commands::ExecuteEffectCommand>(world, [this](WorldRef w, const Commands::ExecuteEffectCommand& cmd)
    {
        std::shared_ptr<FeatureEffects> effectsFeature = GetFeature<FeatureEffects>(w);
        effectsFeature->ExecuteEffect(w, cmd.EffectNode, cmd.EffectId);
    });
}