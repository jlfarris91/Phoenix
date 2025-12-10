
#pragma once

#include "Component.h"
#include "Vitals.h"
#include "FixedPoint/FixedTypes.h"

namespace Phoenix::RTS
{
    struct VitalsComponent : ECS::IComponent
    {
        PHX_ECS_DECLARE_COMPONENT(VitalsComponent)

        Vital<Value> Health;
        Vital<Value> Energy;
        Vital<Value> Shield;
    };
}
