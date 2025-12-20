
#pragma once

#include "PhoenixSim/LDS/LDSObjectModel.h"
#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Data/DataIcon.h"

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

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(CommandButton)
}
