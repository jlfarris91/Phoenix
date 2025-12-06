
#pragma once

#include "DLLExport.h"
#include "LDSObjectModel.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API UnitCargo
    {
        static bool Read(const LDS::LDSReadObjectArgs& context, UnitCargo& outItem);
    };

    struct PHOENIX_RTS_API UnitCargoPtr : LDS::TLDSObjectPtr<UnitCargo>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(UnitCargo);
    };
}
