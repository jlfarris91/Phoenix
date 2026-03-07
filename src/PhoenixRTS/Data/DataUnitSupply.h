
#pragma once

#include "PhoenixSim/LDS/LDSObjectModel.h"
#include "PhoenixRTS/DLLExport.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API UnitSupply
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, UnitSupply& outItem);
    };

    struct PHOENIX_RTS_API UnitSupplyPtr : LDS::TLDSObjectPtr<UnitSupply>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(UnitSupply);
    };

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(UnitSupply)
}
