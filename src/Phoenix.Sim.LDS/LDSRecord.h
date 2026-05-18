#pragma once

#include "LDSValue.h"
#include "Phoenix.Sim/Name.h"

namespace Phoenix::LDS
{
    class PHOENIX_SIM_API LDSRecord
    {
    public:
        LDSRecord() = default;
        LDSRecord(const LDSRecord& other) = default;
        LDSRecord(const FName& objectId, const FName& propertyId, const LDSTypedValue& value);
        LDSRecord(const FName& objectId, const FName& propertyId, const LDSValue& value, ELDSValueType valueType);

        bool IsValid() const;

        uint64 GetId() const;

        const FName& GetObjectId() const;

        const FName& GetPropertyId() const;

        ELDSValueType GetValueType() const;

        LDSTypedValue& GetValue();

        const LDSTypedValue& GetValue() const;

        template <class T>
        T& GetValueAs()
        {
            return *reinterpret_cast<T*>(&Value.Value);
        }

        template <class T>
        const T& GetValueAs() const
        {
            return *reinterpret_cast<const T*>(&Value.Value);
        }

        void SetValue(const LDSTypedValue& value);
        void SetValue(const LDSValue& value, ELDSValueType valueType);

        struct GetItemKey
        {
            uint64 operator()(const LDSRecord& record) const;
        };

    private:

        FName ObjectId;
        FName PropertyId;
        LDSTypedValue Value;

#ifdef DEBUG
        hash64_t RecordId = 0;
#endif
    };
}
