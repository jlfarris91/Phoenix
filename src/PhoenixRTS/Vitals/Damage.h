#pragma once

#include "PhoenixSim/ECS/EntityId.h"
#include "PhoenixSim/FixedPoint/FixedTypes.h"
#include "PhoenixSim/Name.h"

#include "PhoenixRTS/DLLExport.h"

namespace Phoenix::RTS
{
    struct PHOENIX_RTS_API Damage
    {
        PHX_DECLARE_TYPE(Damage)

        FName VitalId;
        ECS::EntityId SourceId;
        Value BaseAmount;
        Value Amount;
        Value ArmorMultiplier;
    };
}

PHX_DEFINE_TYPE(Phoenix::RTS::Damage)
{
    registration
        .Field("Amount",            &RTS::Damage::Amount)
        .Field("ArmorMultiplier",   &RTS::Damage::ArmorMultiplier)
        .Field("BaseAmount",        &RTS::Damage::BaseAmount)
        .Field("SourceId",          &RTS::Damage::SourceId)
        .Field("VitalId",           &RTS::Damage::VitalId);
}