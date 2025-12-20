
#include "PhoenixRTS/Data/DataCommandGridSlot.h"

using namespace Phoenix::RTS::Data;

bool CommandGridSlot::Read(const LDS::LDSReadObjectArgs& args, CommandGridSlot& outItem)
{
    bool success = true;
    return success;
}

CommandGridSlotPtr::CommandGridSlotPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
    , GridRow(Value<uint32>("grid_row"))
    , GridColumn(Value<uint32>("grid_col"))
    , SubMenu(Value<uint32>("sub_menu"))
    , TargetSubMenu(Value<uint32>("target_sub_menu"))
{
}
