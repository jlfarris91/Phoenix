#pragma once

#include "PhoenixSim/ECS/Component.h"
#include "PhoenixSim/ECS/EntityId.h"
#include "PhoenixSim/FixedPoint/FixedVector.h"

#include "PhoenixRTS/DLLExport.h"

namespace Phoenix::RTS
{
    struct PHOENIX_RTS_API EffectComponent : ECS::IComponent
    {
        PHX_DECLARE_TYPE(EffectComponent, ECS::IComponent)

        FName Name;
        ECS::EntityId SourceId;
        ECS::EntityId TargetId;
        Vec2 SourcePos;
        Vec2 TargetPos;
        uint16 RefCount = 0;
        uint16 ChannelingCount = 0;
    };
}

PHX_DEFINE_TYPE(Phoenix::RTS::EffectComponent)
{
    registration
        .Field("Name",              &RTS::EffectComponent::Name)
        .Field("SourceId",          &RTS::EffectComponent::SourceId)
        .Field("TargetId",          &RTS::EffectComponent::TargetId)
        .Field("SourcePos",         &RTS::EffectComponent::SourcePos)
        .Field("TargetPos",         &RTS::EffectComponent::TargetPos)
        .Field("RefCount",          &RTS::EffectComponent::RefCount)
        .Field("ChannelingCount",   &RTS::EffectComponent::ChannelingCount);
}