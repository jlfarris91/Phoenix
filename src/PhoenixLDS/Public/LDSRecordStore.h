
#pragma once

#include <execution>
#include <nlohmann/json.hpp>

#include "DLLExport.h"
#include "Actions.h"
#include "Optional.h"
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
        Time,

        // Localized string
        Text,

        // Asset reference
        Asset,

        // Special types
        Array,
        Object,     // A full object definition.
        ObjectRef,  // A reference to another object in the catalog.
        Expression  // A reference to code.
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
        Time Time;
        bool Bool;
    };

    PHOENIX_LDS_API struct LDSTypedValue
    {
        constexpr LDSTypedValue() = default;

        constexpr LDSTypedValue(ELDSValueType type)
            : Value({ .UInt32 = (uint32)type })
            , Type(type)
        {
        }

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

        constexpr bool IsValid() const
        {
            return !FName::IsNoneOrEmpty(ObjectId);
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

    struct LDSObjectRun
    {
        uint32 Start = 0;
        uint32 End = 0;
    };

    template <class TContainer>
    PHOENIX_LDS_API class TLDSRecordStore
    {
    public:

        static constexpr hash64_t ObjectMask = 0xFFFFFFFF00000000ULL;
        static constexpr hash64_t PropertyMask = 0x00000000FFFFFFFFULL;

        template <class ...TArgs>
        LDSRecord& EmplaceRecord_GetRef(TArgs&& ...args)
        {
            LDSRecord& record = Records.EmplaceBack_GetRef(Forward<TArgs>(args)...);
            return record;
        }

        LDSRecord* FindRecord(hash64_t recordId, hash64_t mask = -1, const LDSObjectRun& hint = {})
        {
            uint32 index = FindIndexOfRecord(recordId, mask, hint);
            return Records.IsValidIndex(index) ? Records[index] : nullptr;
        }

        const LDSRecord* FindRecord(hash64_t recordId, hash64_t mask = -1, const LDSObjectRun& hint = {}) const
        {
            uint32 index = FindIndexOfRecord(recordId, mask, hint);
            return Records.IsValidIndex(index) ? Records[index] : nullptr;
        }

        LDSRecord* FindRecord(const FName& objectId, const FName& propertyId, const LDSObjectRun& hint = {})
        {
            return FindRecord(ToRecordId(objectId, propertyId), -1, hint);
        }

        const LDSRecord* FindRecord(const FName& objectId, const FName& propertyId, const LDSObjectRun& hint = {}) const
        {
            return FindRecord(ToRecordId(objectId, propertyId), -1, hint);
        }

        constexpr bool HasObject(const FName& objectId, const LDSObjectRun& run = {}) const
        {
            return FindRecord(ToRecordId(objectId), ObjectMask, run) != nullptr;
        }

        constexpr bool HasRecord(const FName& objectId, const FName& propertyId, const LDSObjectRun& hint = {}) const
        {
            return FindRecord(ToRecordId(objectId, propertyId), -1, hint) != nullptr;
        }

        template <class TCallback>
        void ForEachRecord(const FName& objectId, const TCallback& callback, const LDSObjectRun& hint = {}) const
        {
            hash64_t recordId = ToRecordId(objectId);
            uint32 sortedLength = SortedLength.GetValue(0);

            // Iterate the sorted range first
            if (sortedLength != 0)
            {
                uint32 index = FindIndexOfRecordInSortedRange(recordId, ObjectMask, hint);

                // Iterate through the sorted records until we hit a new object id.
                for (; index < sortedLength && Records[index].GetObjectId() == objectId; ++index)
                {
                    callback(Records[index]);
                }
            }

            // Then iterate the unsorted range in case any new records were added since the last sort.
            auto unsortedIter = Records.begin() + sortedLength;
            for (; unsortedIter != Records.end(); ++unsortedIter)
            {
                if ((unsortedIter->GetId() & ObjectMask) == recordId)
                {
                    callback(*unsortedIter);
                }
            }
        }

        ELDSValueType GetTypeRecordValueType(const FName& objectId, const FName& propertyId, const LDSObjectRun& hint = {}) const
        {
            auto record = FindRecord(objectId, propertyId, -1, hint);
            return record ? record->GetValueType() : ELDSValueType::Unknown;
        }

        template <class T>
        const T& GetRecordValueAs(const FName& objectId, const FName& propertyId, const T& defaultValue = {}, const LDSObjectRun& hint = {})
        {
            auto record = FindRecord(objectId, propertyId, -1, hint);
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
                RecordSort());

            uint32 i = 0;
            for (; i < Records.Num(); ++i)
            {
                if (!Records[i].IsValid())
                {
                    break;
                }
            }

            Records.SetNum(i);
            SortedLength = i;
        }

    private:

        struct RecordSort
        {
            constexpr bool operator<(const LDSRecord& a, const LDSRecord& b) const
            {
                // Sort invalid records to the back.
                if (!a.IsValid())
                {
                    return false;
                }
                if (!b.IsValid())
                {
                    return true;
                }
                return a.GetId() < b.GetId();
            }
        };

        struct SortedMaskedRecordIdPred
        {
            hash64_t Mask;
            constexpr bool operator<(const LDSRecord& record, hash64_t id) const
            {
                return (record.GetId() & Mask) < id;
            }
        };

        struct UnsortedMaskedRecordIdPred
        {
            hash64_t Mask;
            constexpr bool operator==(const LDSRecord& record, hash64_t id) const
            {
                return (record.GetId() & Mask) == id;
            }
        };

        // Find the index of a record by searching the sorted range followed by the unsorted range.
        uint32 FindIndexOfRecord(hash64_t recordId, const LDSObjectRun& run, hash64_t mask)
        {
            // First try to search the sorted range since it will be O(log N).
            uint32 index = FindIndexOfRecordInSortedRange(recordId, mask, run);
            if (index != Index<uint32>::None)
            {
                return index;
            }

            // Then fallback to searching the unsorted range which is O(N).
            return FindIndexOfRecordInUnsortedRange(recordId, mask);
        }

        // Find the index of a record in the sorted range with an option subset defined by run.
        // This operation is O(log N).
        uint32 FindIndexOfRecordInSortedRange(hash64_t recordId, hash64_t mask, const LDSObjectRun& run)
        {
            if (!SortedLength.IsSet())
            {
                return Index<uint32>::None;
            }

            uint32 sortedLength = SortedLength.Get();

            // If a valid run was specified, try to use that because it should greatly reduce the search area.
            if (run.Start < run.End & run.End <= sortedLength)
            {
                // First search in the sorted range defined by the run.
                uint32 index = FindIndexOfRecordInSortedRun(recordId, mask, run);
                if (index != Index<uint32>::None)
                {
                    return index;
                }

                // Then check the sorted range prior to the run.
                LDSObjectRun beforeRun = { 0, run.Start };
                index = FindIndexOfRecordInSortedRun(recordId, mask, beforeRun);
                if (index != Index<uint32>::None)
                {
                    return index;
                }

                // Then check the sorted range after the run.
                LDSObjectRun afterRun = { run.End, sortedLength };
                return FindIndexOfRecordInSortedRun(recordId, mask, afterRun);
            }

            // If no run was specified then just search the entire sorted range.
            return FindIndexOfRecordInSortedRange(recordId, mask);
        }

        // Search a subset of the sorted range. This operation is O(log N).
        uint32 FindIndexOfRecordInSortedRun(hash64_t recordId, hash64_t mask, const LDSObjectRun& run)
        {
            if (run.Start >= run.End || run.End >= SortedLength.Get())
            {
                // Invalid range.
                return Index<uint32>::None;
            }
            auto begin = Records.begin() + run.Start;
            auto end = Records.begin() + run.End;
            return FindIndexOfRecordInSortedRange(begin, end, recordId, mask);
        }

        // Search the entire sorted range. This operation is O(log N).
        uint32 FindIndexOfRecordInSortedRange(hash64_t recordId, hash64_t mask)
        {
            uint32 sortedLength = SortedLength.Get();
            if (sortedLength == 0)
            {
                // Not sorted yet so there's nothing to search.
                return Index<uint32>::None;
            }
            auto begin = Records.begin();
            auto end = begin + sortedLength;
            return FindIndexOfRecordInSortedRange(begin, end, recordId, mask);
        }

        // Search a subset of the sorted range. This operation is O(log N).
        uint32 FindIndexOfRecordInSortedRange(
            const decltype(TContainer::begin())& begin,
            const decltype(TContainer::end())& end,
            hash64_t recordId,
            hash64_t mask)
        {
            auto iter = std::lower_bound(
                begin,
                end,
                recordId & mask,
                SortedMaskedRecordIdPred { mask });

            // We did not find the record within the sorted range.
            if (iter == end)
            {
                return Index<uint32>::None;
            }

            // We found the record within the sorted range. Return the absolute index.
            return iter - Records.begin();
        }

        // Search the entire unsorted range. This operation is O(N).
        uint32 FindIndexOfRecordInUnsortedRange(hash64_t recordId, hash64_t mask)
        {
            auto iter = std::find(
                Records.begin() + SortedLength.GetValue(0),
                Records.end(),
                recordId & mask,
                UnsortedMaskedRecordIdPred{mask});

            // We did not find the record within the unsorted range.
            if (iter == Records.end())
            {
                return Index<uint32>::None;
            }

            // We found the record within the unsorted range. Return the absolute index.
            return iter - Records.begin();
        }
        
        TContainer Records;
        TOptional<uint32> SortedLength;
    };

    template <size_t N>
    using TFixedRecordStore = TLDSRecordStore<TFixedArray<LDSRecord, N>>;

    using RecordStore = TLDSRecordStore<TArray2<LDSRecord>>;
}
