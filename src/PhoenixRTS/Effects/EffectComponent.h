#pragma once

#include "PhoenixSim/ECS/Component.h"
#include "PhoenixSim/ECS/EntityId.h"
#include "PhoenixSim/FixedPoint/FixedVector.h"

#include "PhoenixRTS/DLLExport.h"

namespace Phoenix::RTS
{
    struct PHOENIX_RTS_API EffectComponent : ECS::IComponent
    {
        PHX_ECS_DECLARE_COMPONENT_BEGIN(EffectComponent)
            PHX_REGISTER_FIELD(FName, Name)
            PHX_REGISTER_FIELD(FName, Name)
            PHX_REGISTER_FIELD(ECS::EntityId, SourceId)
            PHX_REGISTER_FIELD(ECS::EntityId, TargetId)
            PHX_REGISTER_FIELD(Vec2, SourcePos)
            PHX_REGISTER_FIELD(Vec2, TargetPos)
            PHX_REGISTER_FIELD(uint16, RefCount)
            PHX_REGISTER_FIELD(uint16, ChannelingCount)
        PHX_ECS_DECLARE_COMPONENT_END()

        FName Name;
        ECS::EntityId SourceId;
        ECS::EntityId TargetId;
        Vec2 SourcePos;
        Vec2 TargetPos;
        uint16 RefCount = 0;
        uint16 ChannelingCount = 0;
    };
}
