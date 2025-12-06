
#pragma once

#include "DLLExport.h"
#include "LDSObjectModel.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API FogVisibility
    {
        static bool Read(const LDS::LDSReadObjectArgs& context, FogVisibility& outItem);
    };

    struct PHOENIX_RTS_API FogVisibilityPtr : LDS::TLDSObjectPtr<FogVisibility>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(FogVisibility);
    };
}
