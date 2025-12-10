
#include "Data/DataCommandButton.h"

using namespace Phoenix::RTS::Data;

bool CommandButton::Read(const LDS::LDSReadObjectArgs& args, CommandButton& outItem)
{
    bool success = true;
    return success;
}

CommandButtonPtr::CommandButtonPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
    , Icon(Object<IconPtr>("icon"))
    , IsRepeatable(Value<bool>("repeatable"))
{
}
