
#pragma once

#include "Phoenix.Sim/LDS/LDSObjectModel.h"
#include "Phoenix.Sim.RTS/DLLExport.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API UnitInfo
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, UnitInfo& outItem);
    };

    struct PHOENIX_RTS_API UnitInfoPtr : LDS::TLDSObjectPtr<UnitInfo>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(UnitInfo);
    };

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(UnitInfo)
}
