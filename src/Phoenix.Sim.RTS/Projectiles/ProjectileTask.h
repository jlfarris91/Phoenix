#pragma once

#include "ProjectileState.h"
#include "Phoenix.Sim.RTS/DLLExport.h"
#include "Phoenix.Sim.Tasks/TaskBase.h"

namespace Phoenix::RTS
{
    struct PHOENIX_RTS_API ProjectileTask : Tasks::TaskBase
    {
        PHX_DECLARE_TASK(ProjectileTask)

        ProjectileState State;

        void OnCreate(WorldRef world, uint32 context);
        void OnUpdate(WorldRef world, uint32 context);
        void OnFinish(WorldRef world, uint32 context);
    };
}
