
#pragma once

#include "Phoenix.Sim.RTS/Data/DataComponent.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API VitalComponent : Component
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, VitalComponent& outItem);
    };

    struct PHOENIX_RTS_API VitalComponentPtr : ComponentPtr
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(VitalComponent);
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(VitalComponent)
}
