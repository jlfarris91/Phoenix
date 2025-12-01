
#pragma once

#include <execution>
#include <nlohmann/json.hpp>

#include "DLLExport.h"
#include "Actions.h"
#include "Platform.h"
#include "Containers/Array.h"
#include "Containers/FixedArray.h"

namespace Phoenix::LDS
{
    enum class ELDSValueType : uint8
    {
        Unknown,

        // Pod types
        Bool,
        Int32,
        UInt32,
        Name,
        Value,
        Distance,
        Degrees,
        Speed,

        // Special types
        Array,
        Object,
        ObjectRef
    };

    PHOENIX_LDS_API bool TryParse(const PHXString& string, ELDSValueType& outEnum);

    PHOENIX_LDS_API union LDSValue
    {
        int32 Int32;
        uint32 UInt32;
        FName Name;
        Value Value;
        Distance Distance;
        Angle Degrees;
        Speed Speed;
        bool Bool;
    };

    PHOENIX_LDS_API struct LDSTypedValue
    {
        constexpr LDSTypedValue() = default;

        constexpr LDSTypedValue(const LDSValue& value, ELDSValueType type)
            : Value(value)
            , Type(type)
        {
        }

#define DEFINE_LDSTypedValue_CTOR(type, enumType) \
        constexpr LDSTypedValue(type value) \
            : Value{ .enumType = value } \
            , Type(ELDSValueType::enumType) {}

        DEFINE_LDSTypedValue_CTOR(int32, Int32)
        DEFINE_LDSTypedValue_CTOR(uint32, UInt32)
        DEFINE_LDSTypedValue_CTOR(FName, Name)
        DEFINE_LDSTypedValue_CTOR(Value, Value)
        // DEFINE_LDSTypedValue_CTOR(Distance, Distance);
        DEFINE_LDSTypedValue_CTOR(Angle, Degrees)
        DEFINE_LDSTypedValue_CTOR(Speed, Speed)

#undef DEFINE_LDSTypedValue_CTOR

        LDSValue Value = {};
        ELDSValueType Type = ELDSValueType::Unknown;
    };

    PHOENIX_LDS_API class LDSRecord
    {
    public:
        constexpr LDSRecord() = default;
        constexpr LDSRecord(const LDSRecord& other) = default;

        constexpr LDSRecord(const FName& objectId, const FName& propertyId, const LDSTypedValue& value)
            : ObjectId(objectId)
            , PropertyId(propertyId)
            , Value(value)
        {
#if DEBUG
            RecordId = GetId();
#endif
        }

        constexpr LDSRecord(const FName& objectId, const FName& propertyId, const LDSValue& value, ELDSValueType valueType)
            : ObjectId(objectId)
            , PropertyId(propertyId)
            , Value(LDSTypedValue(value, valueType))
        {
#if DEBUG
            RecordId = GetId();
#endif
        }

        constexpr uint64 GetId() const
        {
            return (uint64)(uint32)ObjectId << 32 | (uint32)PropertyId;
        }

        constexpr const FName& GetObjectId() const
        {
            return ObjectId;
        }

        constexpr const FName& GetPropertyId() const
        {
            return PropertyId;
        }

        constexpr LDSTypedValue& GetValue()
        {
            return Value;
        }

        constexpr const LDSTypedValue& GetValue() const
        {
            return Value;
        }

        template <class T>
        constexpr T& GetValueAs()
        {
            return *reinterpret_cast<T*>(&Value.Value);
        }

        template <class T>
        constexpr const T& GetValueAs() const
        {
            return *reinterpret_cast<const T*>(&Value.Value);
        }

        ELDSValueType GetValueType() const
        {
            return Value.Type;
        }

    private:

        FName ObjectId;
        FName PropertyId;
        LDSTypedValue Value;

#if DEBUG
        hash64_t RecordId = 0;
#endif
    };

    template <class TContainer>
    PHOENIX_LDS_API class TRecordStore
    {
    public:

        static constexpr hash64_t ObjectMask = 0xFFFFFFFF00000000ULL;
        static constexpr hash64_t PropertyMask = 0x00000000FFFFFFFFULL;

        template <class ...TArgs>
        LDSRecord& EmplaceRecord_GetRef(TArgs&& ...args)
        {
            LDSRecord& record = Records.EmplaceBack_GetRef(Forward<TArgs>(args)...);
            bSorted = false;
            return record;
        }

        LDSRecord* FindRecord(hash64_t recordId, hash64_t mask = -1)
        {
            if (bSorted)
            {
                auto iter = std::lower_bound(
                    Records.begin(),
                    Records.end(),
                    recordId & mask,
                    [mask](const LDSRecord& record, hash64_t id)
                    {
                        return (record.GetId() & mask) < id;
                    });
                if (iter == Records.end())
                {
                    return nullptr;
                }
                return &*iter;
            }

            for (uint32 i = 0; i < Records.Num(); ++i)
            {
                if ((Records[i].GetId() & mask) == recordId)
                {
                    return &Records[i];
                }
            }

            return nullptr;
        }

        const LDSRecord* FindRecord(hash64_t recordId, hash64_t mask = -1) const
        {
            return const_cast<TRecordStore*>(this)->FindRecord(recordId, mask);
        }

        LDSRecord* FindRecord(const FName& objectId, const FName& propertyId)
        {
            return FindRecord(ToRecordId(objectId, propertyId));
        }

        const LDSRecord* FindRecord(const FName& objectId, const FName& propertyId) const
        {
            return FindRecord(ToRecordId(objectId, propertyId));
        }

        constexpr bool HasObject(const FName& objectId) const
        {
            return FindRecord(ToRecordId(objectId), ObjectMask) != nullptr;
        }

        template <class TCallback>
        void ForEachRecord(const FName& objectId, const TCallback& callback) const
        {
            hash64_t recordId = ToRecordId(objectId);
            if (bSorted)
            {
                auto iter = std::lower_bound(
                    Records.begin(),
                    Records.end(),
                    recordId,
                    [](const LDSRecord& record, hash64_t id)
                    {
                        return (record.GetId() & ObjectMask) < id;
                    });
                if (iter == Records.end())
                {
                    return;
                }

                while (iter != Records.end())
                {
                    callback(*iter);
                    ++iter;

                    // Reached the end of the run
                    if ((iter->GetId() & ObjectMask) != recordId)
                    {
                        break;
                    }
                }
            }

            for (uint32 i = 0; i < Records.Num(); ++i)
            {
                if ((Records[i].GetId() & ObjectMask) == recordId)
                {
                    callback(Records[i]);
                }
            }
        }

        ELDSValueType GetTypeRecordValueType(const FName& objectId, const FName& propertyId) const
        {
            auto record = FindRecord(objectId, propertyId);
            return record ? record->GetValueType() : ELDSValueType::Unknown;
        }

        template <class T>
        const T& GetRecordValueAs(const FName& objectId, const FName& propertyId, const T& defaultValue = {})
        {
            auto record = FindRecord(objectId, propertyId);
            return record ? record->GetValueAs<T>() : defaultValue;
        }

        PHX_FORCE_INLINE static constexpr hash64_t ToRecordId(const FName& objectId, const FName& propertyId = FName::None)
        {
            return (uint64)(uint32)objectId << 32 | (uint32)propertyId;
        }

        void Sort()
        {
            std::sort(
                std::execution::par,
                Records.begin(),
                Records.end(),
                [](const LDSRecord& a, const LDSRecord& b)
                {
                    return a.GetId() < b.GetId();
                });
            bSorted = true;
        }

    private:
        TContainer Records;
        bool bSorted = false;
    };

    template <size_t N>
    using TFixedRecordStore = TRecordStore<TFixedArray<LDSRecord, N>>;

    using RecordStore = TRecordStore<TArray2<LDSRecord>>;
}
