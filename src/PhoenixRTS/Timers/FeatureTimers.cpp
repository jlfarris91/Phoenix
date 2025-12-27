#include "PhoenixRTS/Timers/FeatureTimers.h"

#include "PhoenixSim/Profiling.h"

using namespace Phoenix;
using namespace Phoenix::RTS;

FeatureTimers::FeatureTimers()
{
    FEATURE_SESSION_BLOCK(FeatureTimersDynamicSessionBlock)
    FEATURE_WORLD_BLOCK(FeatureTimersDynamicWorldBlock)
    FEATURE_CHANNEL(FeatureChannels::PreUpdate)
    FEATURE_CHANNEL(FeatureChannels::PreWorldUpdate)
}

FeatureTimers::SessionTimerManager* FeatureTimers::GetSessionTimerManager(SessionRef session)
{
    FeatureTimersDynamicSessionBlock* block = session.GetBlock<FeatureTimersDynamicSessionBlock>();
    return block ? &block->TimerManager : nullptr;
}

const FeatureTimers::SessionTimerManager* FeatureTimers::GetSessionTimerManager(SessionConstRef session)
{
    const FeatureTimersDynamicSessionBlock* block = session.GetBlock<FeatureTimersDynamicSessionBlock>();
    return block ? &block->TimerManager : nullptr;
}

FeatureTimers::SessionTimerManager* FeatureTimers::GetSessionTimerManager(WorldRef world)
{
    TWeakPtr<Phoenix::Session> weakSession = world.GetSession();
    if (auto session = weakSession.lock())
    {
        return GetSessionTimerManager(*session);
    }
    return nullptr;
}

FeatureTimers::WorldTimerManager* FeatureTimers::GetWorldTimerManager(WorldRef world)
{
    FeatureTimersDynamicWorldBlock* block = world.GetBlock<FeatureTimersDynamicWorldBlock>();
    return block ? &block->TimerManager : nullptr;
}

const FeatureTimers::WorldTimerManager* FeatureTimers::GetWorldTimerManager(WorldConstRef world)
{
    const FeatureTimersDynamicWorldBlock* block = world.GetBlock<FeatureTimersDynamicWorldBlock>();
    return block ? &block->TimerManager : nullptr;
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
