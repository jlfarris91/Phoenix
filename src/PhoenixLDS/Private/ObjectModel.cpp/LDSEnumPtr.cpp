#include "ObjectModel/LDSEnumPtr.h"

using namespace Phoenix::LDS;

LDSEnumItemPtr::LDSEnumItemPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSObjectPtr(path, flags)
{
    InitCommon();
}

LDSEnumItemPtr::LDSEnumItemPtr(const LDSRecordPtr& other)
    : LDSObjectPtr(other)
{
    InitCommon();
}

void LDSEnumItemPtr::InitCommon()
{
    Key = LDSObjectPtr::Value<FName>("key");
    Value = LDSObjectPtr::Value("value");
}

LDSEnumPtr::LDSEnumPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSObjectPtr(path, flags)
{
}

LDSEnumPtr::LDSEnumPtr(const LDSRecordPtr& other)
    : LDSObjectPtr(other)
{
}

void LDSEnumPtr::InitCommon()
{
}