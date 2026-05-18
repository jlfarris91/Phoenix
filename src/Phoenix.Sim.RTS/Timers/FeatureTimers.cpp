#include "Phoenix.Sim.RTS/Timers/FeatureTimers.h"

#include "Phoenix/Profiling.h"
#include "Phoenix.Sim/Session.h"

using namespace Phoenix;
using namespace Phoenix::RTS;

void FeatureTimersDynamicBlock::Construct(BlockBufferAllocator& allocator, const Config& config)
{
    TimerManager.Construct(allocator, config.MaxTimers);
}

BlockBufferLayout FeatureTimersDynamicBlock::StaticLayout(const Config& config)
{
    return BlockBufferLayout::For<FeatureTimersDynamicBlock>()
        .Container<FixedTimerManager>("TimerManager", config.MaxTimers);
}

FixedTimerManager* FeatureTimers::GetSessionTimerManager(SessionRef session)
{
    FeatureTimersDynamicBlock* block = session.GetBlock<FeatureTimersDynamicBlock>();
    return block ? &block->TimerManager : nullptr;
}

const FixedTimerManager* FeatureTimers::GetSessionTimerManager(SessionConstRef session)
{
    const FeatureTimersDynamicBlock* block = session.GetBlock<FeatureTimersDynamicBlock>();
    return block ? &block->TimerManager : nullptr;
}

FixedTimerManager* FeatureTimers::GetSessionTimerManager(WorldRef world)
{
    std::weak_ptr<Phoenix::Session> weakSession = world.GetSession();
    if (auto session = weakSession.lock())
    {
        return GetSessionTimerManager(*session);
    }
    return nullptr;
}

FixedTimerManager* FeatureTimers::GetWorldTimerManager(WorldRef world)
{
    FeatureTimersDynamicBlock* block = world.GetBlock<FeatureTimersDynamicBlock>();
    return block ? &block->TimerManager : nullptr;
}

const FixedTimerManager* FeatureTimers::GetWorldTimerManager(WorldConstRef world)
{
    const FeatureTimersDynamicBlock* block = world.GetBlock<FeatureTimersDynamicBlock>();
    return block ? &block->TimerManager : nullptr;
}

void FeatureTimers::OnWorldLayout(const WorldLayoutContext& context, BlockBufferConfigBuilder& builder)
{
    IFeature::OnWorldLayout(context, builder);

    FeatureTimersDynamicBlock::Config dynamicBlockConfig;
    dynamicBlockConfig.MaxTimers = PHX_MAX_WORLD_TIMERS;

    if (const FeatureJsonConfig* featureConfig = context.Config.GetFeatureConfig(GetFeatureId()))
    {
        const nlohmann::json& featureConfigData = featureConfig->GetData();
        dynamicBlockConfig.MaxTimers = featureConfigData.value("max_world_timers", dynamicBlockConfig.MaxTimers);
    }

    builder.RegisterBlockWithAlloc<FeatureTimersDynamicBlock>(EBufferBlockType::Dynamic, dynamicBlockConfig);
}

void FeatureTimers::OnPreUpdate(const FeatureUpdateArgs& args)
{
    IFeature::OnPreUpdate(args);
    TickSessionTimers(*Session);
}

void FeatureTimers::OnPreWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    IFeature::OnPreWorldUpdate(world, args);
    TickWorldTimers(world);
}

void FeatureTimers::TickSessionTimers(SessionRef session)
{
    PHX_PROFILE_ZONE_SCOPED;

    if (auto timerManager = GetSessionTimerManager(session))
    {
        timerManager->Tick(session.GetSimTime());
    }
}

void FeatureTimers::TickWorldTimers(WorldRef world)
{
    PHX_PROFILE_ZONE_SCOPED;

    if (auto timerManager = GetWorldTimerManager(world))
    {
        timerManager->Tick(world.GetSimTime());
    }
}
