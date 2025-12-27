#pragma once

#include "PhoenixSim/Features.h"
#include "PhoenixSim/Session.h"

#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Timers/FixedTimerManager.h"

#ifndef PHX_MAX_SESSION_TIMERS
#define PHX_MAX_SESSION_TIMERS 1024
#endif

#ifndef PHX_MAX_WORLD_TIMERS
#define PHX_MAX_WORLD_TIMERS 8192
#endif

namespace Phoenix::RTS
{
    struct PHOENIX_RTS_API FeatureTimersDynamicSessionBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_DYNAMIC(FeatureTimersDynamicSessionBlock)
        FixedTimerManager<PHX_MAX_SESSION_TIMERS> TimerManager;
    };

    struct PHOENIX_RTS_API FeatureTimersDynamicWorldBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_DYNAMIC(FeatureTimersDynamicWorldBlock)
        FixedTimerManager<PHX_MAX_WORLD_TIMERS> TimerManager;
    };

    class PHOENIX_RTS_API FeatureTimers : IFeature
    {
        PHX_DECLARE_FEATURE_TYPE(FeatureTimers)

    public:

        FeatureTimers();

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
