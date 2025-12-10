
#pragma once

#include "DLLExport.h"
#include "LDSObjectModel.h"

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
}
