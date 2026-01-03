
#pragma once

#include "PhoenixSim/Features.h"
#include "PhoenixSim/Blackboard/FixedBlackboard.h"
#include "PhoenixSim/Containers/BlockBuffer.h"
#include "PhoenixSim/SessionFwd.h"

#ifndef PHX_BLACKBOARD_MAX_GLOBAL_SIZE
#define PHX_BLACKBOARD_MAX_GLOBAL_SIZE 8192
#endif

#ifndef PHX_BLACKBOARD_MAX_WORLD_SIZE
#define PHX_BLACKBOARD_MAX_WORLD_SIZE (16384 * 8)
#endif

namespace Phoenix::Blackboard
{
    using SessionBlackboard = TFixedBlackboard<PHX_BLACKBOARD_MAX_GLOBAL_SIZE>;
    using WorldBlackboard = TFixedBlackboard<PHX_BLACKBOARD_MAX_WORLD_SIZE>;
    
    struct PHOENIX_SIM_API FeatureBlackboardDynamicSessionBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_DYNAMIC(FeatureBlackboardDynamicSessionBlock)
        SessionBlackboard Blackboard;
    };

    struct PHOENIX_SIM_API FeatureBlackboardDynamicWorldBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_DYNAMIC(FeatureBlackboardDynamicWorldBlock)
        WorldBlackboard Blackboard;
    };
    
    class PHOENIX_SIM_API FeatureBlackboard final : public IFeature
    {
        PHX_DECLARE_FEATURE_TYPE(FeatureBlackboard)

    public:

        FeatureBlackboard();

        void OnPostUpdate(const FeatureUpdateArgs& args) override;
        void OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;

        //
        // Session-level blackboard
        //

        static SessionBlackboard& GetGlobalBlackboard(SessionRef session);
        static const SessionBlackboard& GetGlobalBlackboard(SessionConstRef session);

        //
        // World-level blackboard
        //

        static WorldBlackboard& GetBlackboard(WorldRef world);
        static const WorldBlackboard& GetBlackboard(WorldConstRef world);
    };
}
