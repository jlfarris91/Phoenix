
#pragma once

#include "DLLExport.h"
#include "LDSObjectModel.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API Command
    {
        static bool Read(const LDS::LDSReadObjectContext& context, Command& outItem);
    };

    struct PHOENIX_RTS_API CommandPtr : LDS::TLDSObjectPtr<Command>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Command);
    };
}
