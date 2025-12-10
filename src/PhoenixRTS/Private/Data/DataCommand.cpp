
#include "Data/DataCommand.h"

using namespace Phoenix::LDS;
using namespace Phoenix::RTS::Data;

bool Command::Read(const LDSReadObjectArgs& args, Command& outItem)
{
    bool success = true;
    return success;
}

CommandPtr::CommandPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
    , Ability(ObjectRef<LDSObjectRefPtr>("ability"))
    , Button(ObjectRef<CommandButtonPtr>("button"))
    , CommandIndex(Value<uint8>("command_index"))
    , GridSlot(Object<CommandGridSlotPtr>("grid_slot"))
{
}
