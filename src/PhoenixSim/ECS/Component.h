
#pragma once

#include "PhoenixSim/Reflection.h"

namespace Phoenix
{
    namespace ECS
    {
        struct PHOENIX_SIM_API IComponent
        {
            PHX_DECLARE_INTERFACE(IComponent)
        };
    }
}

#define PHX_ECS_DECLARE_COMPONENT_BEGIN(name) PHX_DECLARE_TYPE_WITH_BASE_BEGIN(name, Phoenix::ECS::IComponent)
#define PHX_ECS_DECLARE_COMPONENT_END() PHX_DECLARE_TYPE_WITH_BASE_END()

#define PHX_ECS_DECLARE_COMPONENT(name) \
    PHX_ECS_DECLARE_COMPONENT_BEGIN(name) \
    PHX_ECS_DECLARE_COMPONENT_END()