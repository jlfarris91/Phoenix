
#pragma once

#include "PhoenixSim/ECS/System.h"

#include "PhoenixRTS/DLLExport.h"

namespace Phoenix::RTS
{
    class PHOENIX_RTS_API VitalsSystem : public ECS::ISystem
    {
    public:
        PHX_ECS_DECLARE_SYSTEM_BEGIN(VitalsSystem)
        PHX_ECS_DECLARE_SYSTEM_END()

        void OnWorldUpdate(WorldRef world, const ECS::SystemUpdateArgs& args) override;
    };
}
