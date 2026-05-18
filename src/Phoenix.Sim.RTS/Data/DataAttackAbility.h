
#pragma once

#include "Phoenix.Sim.LDS/LDSObjectModel.h"

#include "Phoenix.Sim.RTS/DLLExport.h"
#include "Phoenix.Sim.RTS/Data/DataAbility.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API AttackAbility : Ability
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, AttackAbility& outItem);
    };

    struct PHOENIX_RTS_API AttackAbilityPtr : AbilityPtr
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(AttackAbility)
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(AttackAbility)
}
