
#pragma once

#include "PhoenixSim/Features.h"
#include "PhoenixSim/ECS/Component.h"
#include "PhoenixSim/ECS/Entity.h"

#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/SmartValues/FixedSmartValueCollection.h"
#include "PhoenixRTS/SmartValues/SmartValue.h"

namespace Phoenix::RTS
{
    struct PHOENIX_RTS_API FeatureSmartValuesDynamicBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_DYNAMIC(FeatureSmartValuesDynamicBlock)

        TFixedSmartValueCollection<1024> SmartValues;
    };

    class FeatureSmartValues : public IFeature
    {
    public:

        PHX_FEATURE_BEGIN(FeatureSmartValues)
            FEATURE_WORLD_BLOCK(FeatureSmartValuesDynamicBlock)
            FEATURE_CHANNEL(FeatureChannels::HandleWorldAction)
        PHX_FEATURE_END()

        static SmartValueAffectorHandle AcquireAffector(WorldRef world);

        static bool ReleaseAffector(WorldRef world, SmartValueAffectorHandle& handle);

        template <class T>
        static T ResolveValue(WorldConstRef world, const SmartValue<T>& value);

    protected:

        bool OnHandleWorldAction(WorldRef world, const FeatureActionArgs& action) override;
    };
}
