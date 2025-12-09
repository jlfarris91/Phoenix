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

LDSEnumTypePtrBase::LDSEnumTypePtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSObjectPtr(path, flags)
{
}

LDSEnumTypePtrBase::LDSEnumTypePtrBase(const LDSEnumTypePtrBase& other)
    : LDSObjectPtr(other)
{
}

LDSEnumTypePtr::LDSEnumTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSEnumTypePtrBase(path, flags)
{
    InitCommon();
}

LDSEnumTypePtr::LDSEnumTypePtr(const LDSEnumTypePtrBase& other)
    : LDSEnumTypePtrBase(other)
{
    InitCommon();
}

LDSEnumTypeItemPtr LDSEnumTypePtr::GetEnumItem(const ILDSQueryContext& context, const FName& key) const
{
    LDSEnumTypeItemPtr itemPtr;
    (void)TryGetEnumItem(context, key, itemPtr);
    return itemPtr;
}

bool LDSEnumTypePtr::TryGetEnumItem(
    const ILDSQueryContext& context,
    const FName& key,
    LDSEnumTypeItemPtr& outItemPtr) const
{
    bool foundItem = false;
    Items.ForEachItem(context, [&](const LDSEnumTypeItemPtr& itemPtr)
    {
        if (itemPtr.Key.GetValue(context, FName::None) == key)
        {
            foundItem = true;
            outItemPtr = itemPtr;
        }
    });
    return foundItem;
}

void LDSEnumTypePtr::InitCommon()
{
    UnderlyingType = Value<ELDSValueType>("underlying_type");
    Items = ObjectArray<LDSEnumTypeItemPtr>("items");
    DefaultValue = Value("default");
}
