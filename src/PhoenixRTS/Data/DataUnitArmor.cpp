
#include "PhoenixRTS/Data/DataUnitArmor.h"

using namespace Phoenix::RTS::Data;

bool UnitArmor::Read(const LDS::LDSReadObjectArgs& args, UnitArmor& outItem)
{
    const LDS::ILDSQueryContext& queryContext = args.GetQueryContext();

    bool success = true;

    UnitArmorPtr dataPtr = args.CreatePtr<UnitArmorPtr>();
    success = dataPtr.Value().TryGetValue(queryContext, outItem.Value) && success;
    success = dataPtr.Icon().TryReadObject(queryContext, outItem.Icon) && success;

    return success;
}

UnitArmorPtr::UnitArmorPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

Phoenix::LDS::TLDSValuePtr<Phoenix::TFixed<12>> UnitArmorPtr::Value() const
{
    return TLDSObjectPtr::Value<FName>("value");
}

IconPtr UnitArmorPtr::Icon() const
{
    return Object<Data::Icon>("icon");
}
