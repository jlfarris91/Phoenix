
#pragma once

#include "PhoenixSim/ECS/Component.h"
#include "PhoenixSim/FixedPoint/FixedTypes.h"

#include "PhoenixRTS/Vitals/Vitals.h"

namespace Phoenix::RTS
{
    struct PHOENIX_RTS_API VitalsComponent : ECS::IComponent
    {
        PHX_ECS_DECLARE_COMPONENT(VitalsComponent)

        Vital<Value> Health;
        Vital<Value> Energy;
        Vital<Value> Shield;
    };
}
