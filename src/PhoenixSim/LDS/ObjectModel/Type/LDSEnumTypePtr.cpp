
#include "PhoenixSim/LDS/ObjectModel/Type/LDSEnumTypePtr.h"

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

LDSEnumTypePtr::LDSEnumTypePtr(const LDSObjectPtr& other)
    : LDSObjectPtr(other)
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
    Items.ForEachItem<LDSEnumTypeItemPtr>(context, [&](uint32, const LDSEnumTypeItemPtr& itemPtr)
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
    Items = ObjectArray("items");
    DefaultValue = Value<FName>("default");
}
