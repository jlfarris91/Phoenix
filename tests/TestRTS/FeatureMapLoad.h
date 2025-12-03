
#pragma once

#include "Features.h"

namespace Phoenix
{
    struct FeatureMapLoad : IFeature
    {
        PHX_FEATURE_BEGIN(FeatureMapLoad)
            FEATURE_CHANNEL(Phoenix::FeatureChannels::WorldInitialize)
        PHX_FEATURE_END()

        void Initialize() override;

        void OnWorldInitialize(WorldRef world) override;
    };
}

