
#pragma once

#include "DLLExport.h"
#include "LDSObjectModel.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API UnitInfo
    {
        static bool Read(const LDS::LDSReadObjectArgs& context, UnitInfo& outItem);
    };

    struct PHOENIX_RTS_API UnitInfoPtr : LDS::TLDSObjectPtr<UnitInfo>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(UnitInfo);
    };
}
