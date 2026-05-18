#pragma once

#include "Phoenix.Sim.ECS/ArchetypeDefinition.h"

namespace Phoenix::ECS
{
    class FixedArchetypeList;

    struct JobBatch
    {
        FixedArchetypeList* List = nullptr;
        uint16 ComponentOffsets[PHX_ECS_ARCHETYPE_MAX_COMPS] = {};
        uint8 NumComponents = 0;
        // Available for jobs to store per-batch data (e.g. pre-partition start offsets).
        uint32 UserData = 0;
    };
}
