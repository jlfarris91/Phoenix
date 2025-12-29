
#include "PhoenixRTS/Data/DataCommandGridSlot.h"

using namespace Phoenix::RTS::Data;

bool CommandGridSlot::Read(const LDS::LDSReadObjectArgs& args, CommandGridSlot& outItem)
{
    bool success = true;
    return success;
}

CommandGridSlotPtr::CommandGridSlotPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

Phoenix::LDS::TLDSValuePtr<unsigned int> CommandGridSlotPtr::GridRow() const
{
    return Value<uint32>("grid_row");
}

Phoenix::LDS::TLDSValuePtr<unsigned int> CommandGridSlotPtr::GridColumn() const
{
    return Value<uint32>("grid_col");
}

Phoenix::LDS::TLDSValuePtr<unsigned int> CommandGridSlotPtr::SubMenu() const
{
    return Value<uint32>("sub_menu");
}

Phoenix::LDS::TLDSValuePtr<unsigned int> CommandGridSlotPtr::TargetSubMenu() const
{
    return Value<uint32>("target_sub_menu");
}
