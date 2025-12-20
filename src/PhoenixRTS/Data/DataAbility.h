
#pragma once

#include "PhoenixSim/LDS/LDSObjectModel.h"

#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Data/DataSmartCast.h"

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
