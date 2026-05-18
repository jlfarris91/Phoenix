#include "Phoenix.Sim/LDS/LDSRecord.h"

Phoenix::LDS::LDSRecord::LDSRecord(const FName& objectId, const FName& propertyId, const LDSTypedValue& value)
    : ObjectId(objectId)
    , PropertyId(propertyId)
    , Value(value)
{
#if DEBUG
    RecordId = GetId();
#endif
}

Phoenix::LDS::LDSRecord::LDSRecord(
    const FName& objectId,
    const FName& propertyId,
    const LDSValue& value,
    ELDSValueType valueType)
    : ObjectId(objectId)
    , PropertyId(propertyId)
    , Value(LDSTypedValue(value, valueType))
{
#if DEBUG
    RecordId = GetId();
#endif
}

bool Phoenix::LDS::LDSRecord::IsValid() const
{
    return !FName::IsNoneOrEmpty(ObjectId);
}

Phoenix::uint64 Phoenix::LDS::LDSRecord::GetId() const
{
    return (uint64)(uint32)ObjectId << 32 | (uint32)PropertyId;
}

const Phoenix::FName& Phoenix::LDS::LDSRecord::GetObjectId() const
{
    return ObjectId;
}

const Phoenix::FName& Phoenix::LDS::LDSRecord::GetPropertyId() const
{
    return PropertyId;
}

Phoenix::LDS::ELDSValueType Phoenix::LDS::LDSRecord::GetValueType() const
{
    return Value.Type;
}

Phoenix::LDS::LDSTypedValue& Phoenix::LDS::LDSRecord::GetValue()
{
    return Value;
}

const Phoenix::LDS::LDSTypedValue& Phoenix::LDS::LDSRecord::GetValue() const
{
    return Value;
}

void Phoenix::LDS::LDSRecord::SetValue(const LDSTypedValue& value)
{
    Value = value;
}

void Phoenix::LDS::LDSRecord::SetValue(const LDSValue& value, ELDSValueType valueType)
{
    Value = LDSTypedValue(value, valueType);
}

Phoenix::uint64 Phoenix::LDS::LDSRecord::GetItemKey::operator()(const LDSRecord& record) const
{
    return record.GetId();
}