
#include "ObjectModel/Type/LDSObjectRefTypePtr.h"

using namespace Phoenix::LDS;

LDSObjectRefTypePtr::LDSObjectRefTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSObjectPtr(path, flags)
{
    InitCommon();
}

LDSObjectRefTypePtr::LDSObjectRefTypePtr(const LDSRecordPtr& other)
    : LDSObjectPtr(other)
{
    InitCommon();
}

void LDSObjectRefTypePtr::InitCommon()
{
    ReferenceType = Value("type");
}