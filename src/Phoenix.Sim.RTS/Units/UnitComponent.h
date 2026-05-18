#pragma once

#include "Phoenix.Sim.ECS/Component.h"

#include "Phoenix.Sim.RTS/DLLExport.h"

namespace Phoenix::RTS
{
    struct PHOENIX_RTS_API UnitComponent : ECS::IComponent
    {
        PHX_ECS_DECLARE_COMPONENT(UnitComponent)

        uint8 OwningPlayer = 0;
        FName UnitData;
    };
}

PHX_DEFINE_TYPE(Phoenix::RTS::UnitComponent)
{
    registration
        .Field("OwningPlayer", &RTS::UnitComponent::OwningPlayer)
        .Field("UnitData",     &RTS::UnitComponent::UnitData);
}
