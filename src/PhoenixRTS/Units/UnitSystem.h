
#pragma once

#include "PhoenixSim/ECS/System.h"

#include "PhoenixRTS/DLLExport.h"

namespace Phoenix::RTS
{
    class PHOENIX_RTS_API UnitSystem : public ECS::ISystem
    {
    public:
        PHX_ECS_DECLARE_SYSTEM_BEGIN(UnitSystem)
        PHX_ECS_DECLARE_SYSTEM_END()

        void OnWorldUpdate(WorldRef world, const ECS::SystemUpdateArgs& args) override;
    };
}
