
#include "Data/DataUnitArmor.h"

using namespace Phoenix::RTS::Data;

bool UnitArmor::Read(const LDS::LDSReadObjectArgs& context, UnitArmor& outItem)
{
    bool success = true;

    UnitArmorPtr dataPtr(context.Path, context.Flags);
    success = dataPtr.Value.TryGetValue(context, outItem.Value) && success;
    success = dataPtr.Icon.TryReadObject(context, outItem.Icon) && success;

    return success;
}

UnitArmorPtr::UnitArmorPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
    , Value(TLDSObjectPtr::Value<FName>("value"))
    , Icon(Object<Data::Icon>("icon"))
{
}
