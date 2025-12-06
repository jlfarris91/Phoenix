
#include "Data/DataBuff.h"

using namespace Phoenix::RTS::Data;

bool Buff::Read(const LDS::LDSReadObjectArgs& context, Buff& outItem)
{
    bool success = true;
    return success;
}

BuffPtr::BuffPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}
