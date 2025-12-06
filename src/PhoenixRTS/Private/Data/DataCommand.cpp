
#include "Data/DataCommand.h"

using namespace Phoenix::RTS::Data;

bool Command::Read(const LDS::LDSReadObjectArgs& args, Command& outItem)
{
    bool success = true;
    return success;
}

CommandPtr::CommandPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}
