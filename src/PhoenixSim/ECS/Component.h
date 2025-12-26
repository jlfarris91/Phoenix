
#pragma once

#include "PhoenixSim/Reflection.h"

namespace Phoenix
{
    namespace ECS
    {
        struct PHOENIX_SIM_API IComponent
        {
            PHX_DECLARE_TYPE(IComponent)
        };
    }
}

#define PHX_ECS_DECLARE_COMPONENT_BEGIN(name) \
    PHX_DECLARE_TYPE_BEGIN(name) \
        PHX_REGISTER_BASE(IComponent)

#define PHX_ECS_DECLARE_COMPONENT_END() PHX_DECLARE_TYPE_END()

#define PHX_ECS_DECLARE_COMPONENT(name) \
    PHX_ECS_DECLARE_COMPONENT_BEGIN(name) \
    PHX_ECS_DECLARE_COMPONENT_END()