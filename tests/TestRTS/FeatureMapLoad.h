
#pragma once

#include <PhoenixSim/Features.h>

namespace Phoenix
{
    struct FeatureMapLoad : IFeature
    {
        PHX_FEATURE_BEGIN(FeatureMapLoad)
            FEATURE_CHANNEL(Phoenix::FeatureChannels::WorldInitialize)
        PHX_FEATURE_END()

        void Initialize(const TSharedPtr<Phoenix::Session>& session) override;

        void OnWorldInitialize(WorldRef world) override;
    };
}

