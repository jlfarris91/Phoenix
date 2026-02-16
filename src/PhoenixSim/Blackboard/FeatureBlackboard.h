
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
    struct PHOENIX_SIM_API FeatureBlackboardBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_WITH_ALLOC(FeatureBlackboardBlock)

        struct Config
        {
            uint32 MaxBlackboardItems;
        };

        FixedBlackboard Blackboard;
    };

    class PHOENIX_SIM_API FeatureBlackboard final : public IFeature
    {
        PHX_DECLARE_FEATURE_TYPE(FeatureBlackboard)

    public:

        FeatureBlackboard();

        void OnWorldLayout(const WorldLayoutContext& context, BlockBufferLayoutBuilder& builder) override;

        void OnPostUpdate(const FeatureUpdateArgs& args) override;
        void OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;

        //
        // Session-level blackboard
        //

        static FixedBlackboard& GetGlobalBlackboard(SessionRef session);
        static const FixedBlackboard& GetGlobalBlackboard(SessionConstRef session);

        //
        // World-level blackboard
        //

        static FixedBlackboard& GetBlackboard(WorldRef world);
        static const FixedBlackboard& GetBlackboard(WorldConstRef world);
    };
}
