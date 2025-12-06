
#pragma once

#include "DLLExport.h"
#include "LDSObjectModel.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API UnitFlags
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, UnitFlags& outItem);
    };

    struct PHOENIX_RTS_API UnitFlagsPtr : LDS::TLDSObjectPtr<UnitFlags>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(UnitFlags);
    };
}
