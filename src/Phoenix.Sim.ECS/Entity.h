
#pragma once

#include "Phoenix.Sim/Name.h"
#include "Phoenix.Sim.ECS/ArchetypeHandle.h"

namespace Phoenix::ECS
{
    struct PHOENIX_SIM_API Entity
    {
        EntityId Id = EntityId::Invalid;
        FName Kind;
    };
}
