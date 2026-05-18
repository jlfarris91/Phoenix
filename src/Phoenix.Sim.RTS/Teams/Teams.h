#pragma once

#include "Phoenix.Sim/Platform.h"
#include "Phoenix.Sim/WorldsFwd.h"

namespace Phoenix::RTS
{
    using TeamMask = uint64;

    struct Teams
    {
        static constexpr TeamMask All = -1;

        static TeamMask AlliesOf(WorldConstRef world, uint8 team);
        static TeamMask EnemiesOf(WorldConstRef world, uint8 team);
    };
}
