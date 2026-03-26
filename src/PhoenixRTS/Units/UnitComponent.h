#pragma once

#include "PhoenixSim/ECS/Component.h"

#include "PhoenixRTS/DLLExport.h"

namespace Phoenix::RTS
{
    struct PHOENIX_RTS_API UnitComponent : ECS::IComponent
    {
        PHX_REFLECT_TYPE(UnitComponent, Phoenix::ECS::IComponent)

        uint8 OwningPlayer = 0;
        FName UnitData;
    };
}
