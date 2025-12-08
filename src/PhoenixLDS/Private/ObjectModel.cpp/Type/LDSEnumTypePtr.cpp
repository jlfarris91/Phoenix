#include "ObjectModel/Type/LDSEnumTypePtr.h"

using namespace Phoenix::LDS;

LDSEnumTypeItemPtr::LDSEnumTypeItemPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSObjectPtr(path, flags)
{
    InitCommon();
}

LDSEnumTypeItemPtr::LDSEnumTypeItemPtr(const LDSRecordPtr& other)
    : LDSObjectPtr(other)
{
    InitCommon();
}

void LDSEnumTypeItemPtr::InitCommon()
{
    Key = LDSObjectPtr::Value<FName>("key");
    Value = LDSObjectPtr::Value("value");
}

LDSEnumTypePtr::LDSEnumTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSObjectPtr(path, flags)
{
    InitCommon();
}

LDSEnumTypePtr::LDSEnumTypePtr(const LDSRecordPtr& other)
    : LDSObjectPtr(other)
{
    InitCommon();
}

void LDSEnumTypePtr::InitCommon()
{
    UnderlyingType = Value<ELDSValueType>("underlying_type");
    Items = Array("items");
    DefaultValue = Value("default");
}