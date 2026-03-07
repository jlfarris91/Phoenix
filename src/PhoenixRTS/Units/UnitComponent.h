#pragma once

#include "PhoenixSim/ECS/Component.h"

#include "PhoenixRTS/DLLExport.h"

namespace Phoenix::RTS
{
    struct PHOENIX_RTS_API UnitComponent : ECS::IComponent
    {
        PHX_ECS_DECLARE_COMPONENT_BEGIN(UnitComponent)
            PHX_REGISTER_FIELD(uint8, OwningPlayer)
            PHX_REGISTER_FIELD(FName, UnitData)
        PHX_ECS_DECLARE_COMPONENT_END()

        uint8 OwningPlayer = 0;
        FName UnitData;
    };
}
