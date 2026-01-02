
#pragma once

#include "PhoenixSim/Name.h"
#include "PhoenixSim/ECS/ArchetypeHandle.h"

namespace Phoenix::ECS
{
    struct PHOENIX_SIM_API Entity
    {
        ArchetypeHandle Handle;
        FName Kind;

        constexpr EntityId GetId() const
        {
            return Handle.GetEntityId();
        }
    };
}
