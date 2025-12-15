#pragma once

#include "Features.h"
#include "FixedTimerManager.h"
#include "Session.h"

#ifndef PHX_MAX_SESSION_TIMERS
#define PHX_MAX_SESSION_TIMERS 1024
#endif

#ifndef PHX_MAX_WORLD_TIMERS
#define PHX_MAX_WORLD_TIMERS 8192
#endif

namespace Phoenix::RTS
{
    struct FeatureTimersDynamicSessionBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_DYNAMIC(FeatureTimersDynamicSessionBlock)
        FixedTimerManager<PHX_MAX_SESSION_TIMERS> TimerManager;
    };

    struct FeatureTimersDynamicWorldBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_DYNAMIC(FeatureTimersDynamicWorldBlock)
        FixedTimerManager<PHX_MAX_WORLD_TIMERS> TimerManager;
    };

    class FeatureTimers : IFeature
    {
        PHX_FEATURE_BEGIN(FeatureTimers)
            FEATURE_SESSION_BLOCK(FeatureTimersDynamicSessionBlock)
            FEATURE_WORLD_BLOCK(FeatureTimersDynamicWorldBlock)
            FEATURE_CHANNEL(FeatureChannels::PreUpdate)
            FEATURE_CHANNEL(FeatureChannels::PreWorldUpdate)
        PHX_FEATURE_END()

    public:

        using SessionTimerManager = decltype(FeatureTimersDynamicSessionBlock::TimerManager);
        using WorldTimerManager = decltype(FeatureTimersDynamicWorldBlock::TimerManager);

        static SessionTimerManager* GetSessionTimerManager(SessionRef session);
        static const SessionTimerManager* GetSessionTimerManager(SessionConstRef session);

        static SessionTimerManager* GetSessionTimerManager(WorldRef world);

        static WorldTimerManager* GetWorldTimerManager(WorldRef world);
        static const WorldTimerManager* GetWorldTimerManager(WorldConstRef world);

    protected:

        void OnPreUpdate(const FeatureUpdateArgs& args) override;
        void OnPreWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;

        static void TickSessionTimers(SessionRef session);
        static void TickWorldTimers(WorldRef world);
    };
}
