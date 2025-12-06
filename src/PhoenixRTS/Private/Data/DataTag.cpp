
#include "Data/DataTag.h"

using namespace Phoenix::RTS::Data;

bool Tag::Read(const LDS::LDSReadObjectArgs& context, Tag& outItem)
{
    bool success = true;
    return success;
}

TagPtr::TagPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}
