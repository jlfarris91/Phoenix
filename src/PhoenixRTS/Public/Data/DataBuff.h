
#pragma once

#include "DLLExport.h"
#include "LDSObjectModel.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API Buff
    {
        static bool Read(const LDS::LDSReadObjectContext& context, Buff& outItem);
    };

    struct PHOENIX_RTS_API BuffPtr : LDS::TLDSObjectPtr<Buff>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Buff);
    };
}
