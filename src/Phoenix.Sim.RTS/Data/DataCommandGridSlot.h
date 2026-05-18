
#pragma once

#include "Phoenix.Sim/LDS/LDSObjectModel.h"
#include "Phoenix.Sim.RTS/DLLExport.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API CommandGridSlot
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, CommandGridSlot& outItem);
    };

    struct PHOENIX_RTS_API CommandGridSlotPtr : LDS::TLDSObjectPtr<CommandGridSlot>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(CommandGridSlot);

        LDS::TLDSValuePtr<uint32> GridRow() const;
        LDS::TLDSValuePtr<uint32> GridColumn() const;
        LDS::TLDSValuePtr<uint32> SubMenu() const;
        LDS::TLDSValuePtr<uint32> TargetSubMenu() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(CommandGridSlot)
}
