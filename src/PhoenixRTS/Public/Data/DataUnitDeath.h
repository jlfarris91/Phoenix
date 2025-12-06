
#pragma once

#include "DLLExport.h"
#include "LDSObjectModel.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API UnitDeath
    {
        static bool Read(const LDS::LDSReadObjectArgs& context, UnitDeath& outItem);
    };

    struct PHOENIX_RTS_API UnitDeathPtr : LDS::TLDSObjectPtr<UnitDeath>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(UnitDeath);
    };
}
