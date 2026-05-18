
#pragma once

#include "PhoenixSim/LDS/LDSObjectModel.h"
#include "PhoenixRTS/DLLExport.h"

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

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(UnitBuild)
}
