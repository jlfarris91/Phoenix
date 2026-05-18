#pragma once

#include "EffectId.h"
#include "Phoenix.Sim/ECS/Component.h"

#include "Phoenix.Sim.RTS/DLLExport.h"

namespace Phoenix::RTS
{
    struct PHOENIX_RTS_API PeriodicEffectComponent : ECS::IComponent
    {
        PHX_ECS_DECLARE_COMPONENT(PeriodicEffectComponent)

        EffectNodeId EffectNode;
        Time PeriodicEffectTime;
        FName PeriodicEffectId;
        Time NextPeriodicEffectTime;
    };
}
