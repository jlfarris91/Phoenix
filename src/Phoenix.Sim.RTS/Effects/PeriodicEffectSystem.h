
#pragma once

#include "Phoenix.Sim.ECS/System.h"

#include "Phoenix.Sim.RTS/DLLExport.h"

namespace Phoenix::RTS
{
    class PHOENIX_RTS_API PeriodicEffectSystem : public ECS::ISystem
    {
    public:
        PHX_ECS_DECLARE_SYSTEM(PeriodicEffectSystem)

        void OnWorldInitialize(WorldRef world) override;
    };
}
