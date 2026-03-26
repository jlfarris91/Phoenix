#pragma once

#include "PhoenixSim/ECS/Component.h"
#include "PhoenixSim/ECS/EntityId.h"
#include "PhoenixSim/FixedPoint/FixedVector.h"

#include "PhoenixRTS/DLLExport.h"

namespace Phoenix::RTS
{
    struct PHOENIX_RTS_API EffectComponent : ECS::IComponent
    {
        PHX_REFLECT_TYPE(EffectComponent, Phoenix::ECS::IComponent)

        FName Name;
        ECS::EntityId SourceId;
        ECS::EntityId TargetId;
        Vec2 SourcePos;
        Vec2 TargetPos;
        uint16 RefCount = 0;
        uint16 ChannelingCount = 0;
    };
}
