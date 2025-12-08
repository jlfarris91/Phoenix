
#include "ObjectModel/Type/LDSObjectTypePtr.h"

using namespace Phoenix::LDS;

LDSObjectTypePtr::LDSObjectTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSObjectPtr(path, flags)
{
}

LDSObjectTypePtr::LDSObjectTypePtr(const LDSRecordPtr& other)
    : LDSObjectPtr(other)
{
}

void LDSObjectTypePtr::InitCommon()
{
    Default = Object("default");
}
