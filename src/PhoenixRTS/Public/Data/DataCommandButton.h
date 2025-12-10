
#pragma once

#include "DataIcon.h"
#include "DLLExport.h"
#include "LDSObjectModel.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API CommandButton
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, CommandButton& outItem);
    };

    struct PHOENIX_RTS_API CommandButtonPtr : LDS::TLDSObjectPtr<CommandButton>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(CommandButton);

        IconPtr Icon;
        LDS::TLDSValuePtr<bool> IsRepeatable;
    };
}
