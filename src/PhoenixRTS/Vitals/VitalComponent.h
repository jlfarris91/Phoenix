
#pragma once

#include "PhoenixSim/ECS/Component.h"

#include "PhoenixRTS/Vitals/Vitals.h"

#ifndef PHX_RTS_MAX_VITALS
#define PHX_RTS_MAX_VITALS 3
#endif

namespace Phoenix::RTS
{
    struct PHOENIX_RTS_API VitalsComponent : ECS::IComponent
    {
        PHX_ECS_DECLARE_COMPONENT(VitalsComponent)

        struct VitalEntry
        {
            FName Id;
            Vital Vital;
        };

        VitalEntry Vitals[PHX_RTS_MAX_VITALS];
        uint8 VitalCount = 0;
    };
}
