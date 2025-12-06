
#pragma once

#include "DLLExport.h"
#include "LDSObjectModel.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API UnitBuild
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, UnitBuild& outItem);
    };

    struct PHOENIX_RTS_API UnitBuildPtr : LDS::TLDSObjectPtr<UnitBuild>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(UnitBuild);
    };
}
