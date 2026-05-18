
#pragma once

#include "Phoenix.Sim/LDS/LDSObjectModel.h"
#include "Phoenix.Sim.RTS/DLLExport.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API Footprint
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, Footprint& outItem);
    };

    struct PHOENIX_RTS_API FootprintPtr : LDS::TLDSObjectPtr<Footprint>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Footprint);
        LDS::NamePtr Asset() const;
        LDS::BoolPtr Snap() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(Footprint)
}
