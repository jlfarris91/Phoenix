
#include "Phoenix.Sim.RTS/Effects/PeriodicEffectSystem.h"

#include "Phoenix.Sim.RTS/ECS/ECSCommands.h"
#include "Phoenix.Sim.ECS/FeatureECS.h"
#include "Phoenix.Sim.ECS/SystemJob.h"
#include "Phoenix.Sim.LDS/FeatureLDS.h"
#include "Phoenix/Profiling.h"
#include "Phoenix.Sim/Session.h"

#include "Phoenix.Sim.RTS/Effects/FeatureEffects.h"
#include "Phoenix.Sim.RTS/Effects/PeriodicEffectComponent.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

namespace
{
    struct PeriodicEffectSystemJob : IJob<PeriodicEffectComponent&>
    {
        const char* GetName() const override { return "Effects.PeriodicEffectSystemJob"; }

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