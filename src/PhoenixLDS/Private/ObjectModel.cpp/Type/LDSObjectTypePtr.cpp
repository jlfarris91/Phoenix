
#include "ObjectModel/Type/LDSObjectTypePtr.h"

using namespace Phoenix::LDS;

LDSObjectTypePtr::LDSObjectTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSObjectPtr(path, flags)
{
    InitCommon();
}

LDSObjectTypePtr::LDSObjectTypePtr(const LDSRecordPtr& other)
    : LDSObjectPtr(other)
{
    InitCommon();
}

void LDSObjectTypePtr::InitCommon()
{
    Default = Object("default");
}
