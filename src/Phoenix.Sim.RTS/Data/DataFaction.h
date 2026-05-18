
#pragma once

#include "Phoenix.Sim/LDS/LDSObjectModel.h"
#include "Phoenix.Sim.RTS/DLLExport.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API Faction
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, Faction& outItem);
    };

    struct PHOENIX_RTS_API FactionPtr : LDS::TLDSObjectPtr<Faction>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Faction);
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(Faction)
}
