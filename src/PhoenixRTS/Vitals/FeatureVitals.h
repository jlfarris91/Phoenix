
#pragma once

#include "PhoenixSim/Features.h"

#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Commands/Commands.h"

namespace Phoenix::RTS
{
    struct Damage;
    class VitalsSystem;

    class PHOENIX_RTS_API FeatureVitals : public IFeature
    {
        PHX_FEATURE_BEGIN(FeatureVitals)
        PHX_FEATURE_END()

    public:

        static bool ApplyDamage(WorldRef world, ECS::EntityId target, const Damage& damage);

    protected:

        void Initialize() override;
        void Shutdown() override;

        TSharedPtr<VitalsSystem> VitalsSystem;
    };
}
