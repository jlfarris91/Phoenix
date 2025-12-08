#include "ObjectModel/Type/LDSNumericTypePtr.h"

using namespace Phoenix::LDS;

LDSNumericTypePtr::LDSNumericTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSObjectPtr(path, flags)
{
    InitCommon();
}

LDSNumericTypePtr::LDSNumericTypePtr(const LDSRecordPtr& other)
    : LDSObjectPtr(other)
{
    InitCommon();
}

void LDSNumericTypePtr::InitCommon()
{
    DefaultValue = Value("default");
    MinValue = Value("min");
    MaxValue = Value("max");
}
