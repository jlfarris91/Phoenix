#pragma once

#include "PhoenixRTS/Effects/EffectId.h"
#include "PhoenixRTS/Orders/Commands.h"
#include "PhoenixRTS/Units/UnitId.h"
#include "PhoenixSim/Name.h"

namespace Phoenix::ECS::Commands
{
    struct RequestAcquireOrder
    {
        static constexpr FName StaticId = "RTS_RequestAcquireOrder"_n;
        RTS::UnitId Target;
        RTS::AcquireRequest Request;
    };

    struct ExecuteEffectCommand
    {
        static constexpr FName StaticId = "RTS_ExecuteEffect"_n;
        RTS::EffectNodeId EffectNode;
        FName EffectId;
    };
}
