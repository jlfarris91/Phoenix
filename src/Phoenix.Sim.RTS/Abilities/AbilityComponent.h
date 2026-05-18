#pragma once

#include "Phoenix.Sim.ECS/Component.h"

#include "Phoenix.Sim.RTS/DLLExport.h"

namespace Phoenix::RTS
{
    struct PHOENIX_RTS_API IAbilityComponent : ECS::IComponent
    {
        PHX_DECLARE_TYPE(IAbilityComponent, ECS::IComponent)
    };
}