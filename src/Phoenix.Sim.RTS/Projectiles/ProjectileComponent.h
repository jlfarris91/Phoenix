#pragma once

#include "Phoenix.Sim.ECS/Component.h"
#include "Phoenix/FixedPoint/FixedTypes.h"
#include "Phoenix/FixedPoint/FixedVector.h"

#include "Phoenix.Sim.RTS/DLLExport.h"
#include "Phoenix.Sim.RTS/Effects/EffectId.h"
#include "Phoenix.Sim.ECS/EntityId.h"

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

        EffectNodeId EffectParent;
        FName ImpactEffectId;
    };
}
