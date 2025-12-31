#pragma once

#include "PhoenixSim/ECS/Component.h"
#include "PhoenixSim/FixedPoint/FixedTypes.h"

#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Effects/EffectId.h"
#include "PhoenixSim/ECS/EntityId.h"

namespace Phoenix::RTS
{
    struct PHOENIX_RTS_API ProjectileComponent : ECS::IComponent
    {
        PHX_ECS_DECLARE_COMPONENT(ProjectileComponent)

        ECS::EntityId Owner;
        FName ProjectileDataId;

        Vec2 LaunchPos;
        ECS::EntityId TargetEntity;
        Vec2 TargetPos;

        EffectNodeId EffectOwner;
        Time NextPeriodic;
        FName PeriodicEffectId;
    };
}
