
#include "PhoenixRTS/Data/DataCommand.h"

using namespace Phoenix::LDS;
using namespace Phoenix::RTS::Data;

bool Command::Read(const LDSReadObjectArgs& args, Command& outItem)
{
    bool success = true;
    return success;
}

CommandPtr::CommandPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

LDSObjectRefPtr CommandPtr::Ability() const
{
    return ObjectRef<LDSObjectRefPtr>("ability");
}

TLDSObjectRefPtr<CommandButton> CommandPtr::Button() const
{
    return ObjectRef<CommandButtonPtr>("button");
}

TLDSValuePtr<unsigned char> CommandPtr::CommandIndex() const
{
    return Value<uint8>("command_index");
}

CommandGridSlotPtr CommandPtr::GridSlot() const
{
    return Object<CommandGridSlotPtr>("grid_slot");
}
