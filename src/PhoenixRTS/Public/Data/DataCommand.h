
#pragma once

#include "DLLExport.h"
#include "LDSObjectModel.h"
#include "DataCommandButton.h"
#include "DataCommandGridSlot.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API Command
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, Command& outItem);
    };

    struct PHOENIX_RTS_API CommandPtr : LDS::TLDSObjectPtr<Command>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Command);

        LDS::LDSObjectRefPtr Ability;
        LDS::TLDSObjectRefPtr<CommandButton> Button;
        LDS::TLDSValuePtr<uint8> CommandIndex;
        CommandGridSlotPtr GridSlot;
    };
}
