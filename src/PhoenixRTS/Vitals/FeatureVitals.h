
#pragma once

#include "PhoenixSim/ECS/EntityId.h"
#include "PhoenixSim/Features.h"

#include "PhoenixRTS/DLLExport.h"

namespace Phoenix::RTS
{
    struct Damage;
    class VitalsSystem;

    class PHOENIX_RTS_API FeatureVitals : public IFeature
    {
        PHX_DECLARE_FEATURE_TYPE(FeatureVitals)

    public:

        static bool ApplyDamage(WorldRef world, ECS::EntityId target, const Damage& damage);

    protected:

        void Initialize(const TSharedPtr<Phoenix::Session>& session) override;
        void Shutdown() override;

        TSharedPtr<VitalsSystem> VitalsSystem;
    };
}
