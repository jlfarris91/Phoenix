
#pragma once

#include "DLLExport.h"
#include "LDSObjectModel.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API UnitVision
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, UnitVision& outItem);
    };

    struct PHOENIX_RTS_API UnitVisionPtr : LDS::TLDSObjectPtr<UnitVision>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(UnitVision);
    };
}
