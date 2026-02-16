#pragma once

#include "PhoenixSim/Features.h"
#include "PhoenixSim/SessionFwd.h"

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
    struct PHOENIX_RTS_API FeatureTimersDynamicBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_WITH_ALLOC(FeatureTimersDynamicBlock)

        struct Config
        {
            uint32 MaxTimers;
        };

        FixedTimerManager TimerManager;
    };

    class PHOENIX_RTS_API FeatureTimers : IFeature
    {
        PHX_DECLARE_FEATURE_TYPE(FeatureTimers)

    public:

        FeatureTimers();

        static FixedTimerManager* GetSessionTimerManager(SessionRef session);
        static const FixedTimerManager* GetSessionTimerManager(SessionConstRef session);

        static FixedTimerManager* GetSessionTimerManager(WorldRef world);

        static FixedTimerManager* GetWorldTimerManager(WorldRef world);
        static const FixedTimerManager* GetWorldTimerManager(WorldConstRef world);

    protected:

        void OnWorldLayout(const WorldLayoutContext& context, BlockBufferLayoutBuilder& builder) override;
        void OnPreUpdate(const FeatureUpdateArgs& args) override;
        void OnPreWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;

        static void TickSessionTimers(SessionRef session);
        static void TickWorldTimers(WorldRef world);
    };
}
