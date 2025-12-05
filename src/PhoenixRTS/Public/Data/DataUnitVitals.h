
#pragma once

#include "DLLExport.h"
#include "LDSObjectModel.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API UnitVitals
    {
        static bool Read(const LDS::LDSReadObjectContext& context, UnitVitals& outItem);
    };

    struct PHOENIX_RTS_API UnitVitalsPtr : LDS::TLDSObjectPtr<UnitVitals>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(UnitVitals);
    };
}
