
#pragma once

#include "Phoenix.Sim.LDS/LDSObjectModel.h"

#include "Phoenix.Sim.RTS/DLLExport.h"
#include "Phoenix.Sim.RTS/Data/DataSmartCast.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API Ability
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, Ability& outItem);
    };

    struct PHOENIX_RTS_API AbilityPtr : LDS::TLDSObjectPtr<Ability>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Ability)

        SmartCastPtr SmartCast() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(Ability)
}
