#pragma once

#include "PhoenixSim/ECS/Component.h"

#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Data/DataProjectileMovement.h"
#include "PhoenixSim/ECS/EntityId.h"

namespace Phoenix::RTS
{
    enum class PHOENIX_RTS_API EProjectileType : uint8
    {
        
    };

    enum class PHOENIX_RTS_API EProjectileFlags : uint8
    {
        None = 0
    };
    
    struct PHOENIX_RTS_API ProjectileComponent : ECS::IComponent
    {
        PHX_ECS_DECLARE_COMPONENT(ProjectileComponent)

        ECS::EntityId Owner;
        FName ProjectileDataId;
    };
}
