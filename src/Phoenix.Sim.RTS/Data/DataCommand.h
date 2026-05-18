
#pragma once

#include "PhoenixSim/LDS/LDSObjectModel.h"

#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Data/DataCommandButton.h"
#include "PhoenixRTS/Data/DataCommandGridSlot.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API Command
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, Command& outItem);
    };

    struct PHOENIX_RTS_API CommandPtr : LDS::TLDSObjectPtr<Command>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Command);

        LDS::LDSObjectRefPtr Ability() const;
        LDS::TLDSObjectRefPtr<CommandButton> Button() const;
        LDS::TLDSValuePtr<uint8> CommandIndex() const;
        CommandGridSlotPtr GridSlot() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(Command)
}
