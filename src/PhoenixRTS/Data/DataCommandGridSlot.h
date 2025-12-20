
#pragma once

#include "PhoenixSim/LDS/LDSObjectModel.h"
#include "PhoenixRTS/DLLExport.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API CommandGridSlot
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, CommandGridSlot& outItem);
    };

    struct PHOENIX_RTS_API CommandGridSlotPtr : LDS::TLDSObjectPtr<CommandGridSlot>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(CommandGridSlot);

        LDS::TLDSValuePtr<uint32> GridRow;
        LDS::TLDSValuePtr<uint32> GridColumn;
        LDS::TLDSValuePtr<uint32> SubMenu;
        LDS::TLDSValuePtr<uint32> TargetSubMenu;
    };

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(CommandGridSlot)
}
