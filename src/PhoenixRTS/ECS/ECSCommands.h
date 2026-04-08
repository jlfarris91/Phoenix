#pragma once

#include "PhoenixRTS/Orders/Commands.h"
#include "PhoenixRTS/Units/UnitId.h"
#include "PhoenixSim/Name.h"

namespace Phoenix::RTS::Commands
{
    struct RequestAcquireOrder
    {
        static constexpr FName StaticId = "RTS_RequestAcquireOrder"_n;
        UnitId Target;
        AcquireRequest Request;
    };
}
