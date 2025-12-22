
#pragma once

#include "PhoenixSim/ECS/Component.h"
#include "PhoenixSim/FixedPoint/FixedTypes.h"

#include "PhoenixRTS/Vitals/Vitals.h"

namespace Phoenix::RTS
{
    struct PHOENIX_RTS_API HealthComponent : ECS::IComponent
    {
        PHX_ECS_DECLARE_COMPONENT(HealthComponent)
        Vital<Value> Health;
    };
}
