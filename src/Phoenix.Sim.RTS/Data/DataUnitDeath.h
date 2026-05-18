
#pragma once

#include "Phoenix.Sim/LDS/LDSObjectModel.h"
#include "Phoenix.Sim.RTS/DLLExport.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API UnitDeath
    {
        Time ExpirationTime;
        static bool Read(const LDS::LDSReadObjectArgs& args, UnitDeath& outItem);
    };

    struct PHOENIX_RTS_API UnitDeathPtr : LDS::TLDSObjectPtr<UnitDeath>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(UnitDeath);

        LDS::TimePtr ExpirationTime() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(UnitDeath)
}
