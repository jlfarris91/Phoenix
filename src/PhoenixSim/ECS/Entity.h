
#pragma once

#include "PhoenixSim/Name.h"
#include "PhoenixSim/ECS/ArchetypeHandle.h"

namespace Phoenix::ECS
{
    struct PHOENIX_SIM_API Entity
    {
        EntityId Id = EntityId::Invalid;
        FName Kind;
    };
}
