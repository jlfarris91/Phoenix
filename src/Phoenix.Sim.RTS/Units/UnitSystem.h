
#pragma once

#include "Phoenix.Sim.ECS/System.h"

#include "Phoenix.Sim.RTS/DLLExport.h"

namespace Phoenix::RTS
{
    class PHOENIX_RTS_API UnitSystem : public ECS::ISystem
    {
    public:
        PHX_ECS_DECLARE_SYSTEM(UnitSystem)

        void OnWorldInitialize(WorldRef world) override;
    };
}
