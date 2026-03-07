
#pragma once

#include "PhoenixSim/LDS/LDSObjectModel.h"
#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Data/DataAbility.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API MoveAbility : Ability
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, MoveAbility& outItem);
    };

    struct PHOENIX_RTS_API MoveAbilityPtr : AbilityPtr
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(MoveAbility)
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(MoveAbility)
}
