#pragma once

#include "PhoenixSim/ECS/EntityId.h"
#include "PhoenixSim/FixedPoint/FixedTypes.h"
#include "PhoenixSim/Name.h"

#include "PhoenixRTS/DLLExport.h"

namespace Phoenix::RTS
{
    struct PHOENIX_RTS_API Damage
    {
        FName VitalId;
        ECS::EntityId SourceId;
        Value BaseAmount;
        Value Amount;
        Value ArmorMultiplier;
    };
}
