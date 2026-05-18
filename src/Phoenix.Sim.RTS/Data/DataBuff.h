
#pragma once

#include "Phoenix.Sim.RTS/DLLExport.h"
#include "Phoenix.Sim.LDS/LDSObjectModel.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API Buff
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, Buff& outItem);
    };

    struct PHOENIX_RTS_API BuffPtr : LDS::TLDSObjectPtr<Buff>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Buff);
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(Buff)
}
