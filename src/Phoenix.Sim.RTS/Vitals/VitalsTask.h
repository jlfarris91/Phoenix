#pragma once

#include "Phoenix.Sim.RTS/DLLExport.h"
#include "Phoenix.Sim/Tasks/TaskBase.h"

namespace Phoenix::RTS
{
    struct PHOENIX_RTS_API VitalsTask : Tasks::TaskBase
    {
        PHX_DECLARE_TASK(VitalsTask)

        void OnCreate(WorldRef world, uint32 context);
        void OnUpdate(WorldRef world, uint32 context);
    };
}
