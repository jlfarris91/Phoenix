
#pragma once

#include "PhoenixSim/Reflection/Reflection.h"

namespace Phoenix
{
    namespace ECS
    {
        struct PHOENIX_SIM_API IComponent
        {
            PHX_ENABLE_TYPE(IComponent)
        };
    }
}

#define PHX_ECS_DECLARE_COMPONENT(name) PHX_ENABLE_TYPE(name, Phoenix::ECS::IComponent)