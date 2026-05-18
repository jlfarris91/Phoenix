
#pragma once

#include "Phoenix.Sim/LDS/LDSObjectModel.h"

#include "Phoenix.Sim.RTS/DLLExport.h"
#include "Phoenix.Sim.RTS/Data/DataCommandButton.h"
#include "Phoenix.Sim.RTS/Data/DataCommandGridSlot.h"

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
