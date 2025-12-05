
#pragma once

#include "DLLExport.h"
#include "LDSObjectModel.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API UnitEffects
    {
        static bool Read(const LDS::LDSReadObjectContext& context, UnitEffects& outItem);
    };

    struct PHOENIX_RTS_API UnitEffectsPtr : LDS::TLDSObjectPtr<UnitEffects>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(UnitEffects);
    };
}
