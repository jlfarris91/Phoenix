#pragma once

#include "Phoenix.Sim.ECS/Component.h"
#include "Phoenix.Sim.ECS/EntityId.h"
#include "Phoenix/FixedPoint/FixedVector.h"

#include "Phoenix.Sim.RTS/DLLExport.h"

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