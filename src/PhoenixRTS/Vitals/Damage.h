#pragma once

#include "PhoenixSim/ECS/EntityId.h"
#include "PhoenixSim/FixedPoint/FixedTypes.h"
#include "PhoenixSim/Name.h"
#include "PhoenixSim/Reflection/Reflection.h"

#include "PhoenixRTS/DLLExport.h"

namespace Phoenix::RTS
{
    struct PHOENIX_RTS_API Damage
    {
        PHX_DECLARE_TYPE(Damage)
        FName VitalId;
        ECS::EntityId SourceId;
        Value BaseAmount;
        Value Amount;
        Value ArmorMultiplier;
    };
}
