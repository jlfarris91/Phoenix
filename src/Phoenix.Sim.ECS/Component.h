
#pragma once

#include "Phoenix/Reflection/Registration.h"

namespace Phoenix
{
    namespace ECS
    {
        struct PHOENIX_SIM_API IComponent
        {
        };
    }
}

#define PHX_ECS_DECLARE_COMPONENT(name) PHX_DECLARE_TYPE(name, Phoenix::ECS::IComponent)