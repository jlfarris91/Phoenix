
#pragma once

#include "DLLExport.h"
#include "LDSObjectModel.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API Faction
    {
        static bool Read(const LDS::LDSReadObjectContext& context, Faction& outItem);
    };

    struct PHOENIX_RTS_API FactionPtr : LDS::TLDSObjectPtr<Faction>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Faction);
    };
}
