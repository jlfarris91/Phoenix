#pragma once

#include "Phoenix.Sim.RTS/Effects/EffectId.h"
#include "Phoenix.Sim.RTS/Orders/Commands.h"
#include "Phoenix.Sim.RTS/Units/UnitId.h"
#include "Phoenix.Sim/Name.h"

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
