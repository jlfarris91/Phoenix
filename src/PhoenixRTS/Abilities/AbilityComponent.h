#pragma once

#include "PhoenixSim/ECS/Component.h"

#include "PhoenixRTS/DLLExport.h"

namespace Phoenix::RTS
{
    struct PHOENIX_RTS_API IAbilityComponent : ECS::IComponent
    {
        PHX_DECLARE_INTERFACE_WITH_BASE(IAbilityComponent, ECS::IComponent)
    };
}