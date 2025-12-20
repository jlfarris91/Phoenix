
#pragma once

#include <execution>

#include "PhoenixSim/LDS/LDSValue.h"
#include "PhoenixSim/Containers/Array.h"
#include "PhoenixSim/Containers/FixedArray.h"

namespace Phoenix::LDS
{
    class PHOENIX_SIM_API LDSRecord
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
    class TLDSRecordStore
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
            return Records.IsValidIndex(index) ? &Records[index] : nullptr;
        }

        const LDSRecord* FindRecord(hash64_t recordId, hash64_t mask = -1, const LDSObjectRun& hint = {}) const
        {
            uint32 index = FindIndexOfRecord(recordId, mask, hint);
            return Records.IsValidIndex(index) ? &Records[index] : nullptr;
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

            // Iterate the sorted range first
            if (SortedNum != 0)
            {
                uint32 index = FindIndexOfRecordInSortedRange(recordId, ObjectMask, hint);

                // Iterate through the sorted records until we hit a new object id.
                for (; index < SortedNum && Records[index].GetObjectId() == objectId; ++index)
                {
                    callback(Records[index]);
                }
            }

            // Then iterate the unsorted range in case any new records were added since the last sort.
            auto unsortedIter = Records.begin() + SortedNum;
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
            auto record = FindRecord(objectId, propertyId, hint);
            return record ? record->GetValueType() : ELDSValueType::Unknown;
        }

        template <class T>
        const T& GetRecordValueAs(const FName& objectId, const FName& propertyId, const T& defaultValue = {}, const LDSObjectRun& hint = {})
        {
            auto record = FindRecord(objectId, propertyId, hint);
            return record ? record->GetValueAs<T>() : defaultValue;
        }

        PHX_FORCE_INLINE static constexpr hash64_t ToRecordId(const FName& objectId, const FName& propertyId = FName::None)
        {
            return (uint64)(uint32)objectId << 32 | (uint32)propertyId;
        }

        LDSObjectRun FindSortedRecordRun(const FName& objectId)
        {
            hash64_t recordId = ToRecordId(objectId);
            uint32 index = FindIndexOfRecordInSortedRange(recordId, ObjectMask);
            if (index == Index<uint32>::None)
            {
                return {};
            }
            uint32 start = index;
            uint32 end = index;
            while (end < SortedNum && (Records[end].GetId() & ObjectMask) == recordId)
            {
                ++end;
            }
            return LDSObjectRun{start, end};
        }

        void Sort()
        {
            std::sort(
                std::execution::par,
                Records.begin(),
                Records.end(),
                RecordSort());

            auto firstInvalidRecord = std::find_if(
                Records.begin(),
                Records.end(),
                InvalidRecordIdPred());

            uint32 sortedNum = (uint32)(firstInvalidRecord - Records.begin());

            Records.SetNum(sortedNum);
            SortedNum = sortedNum;
        }

    private:

        struct InvalidRecordIdPred
        {
            bool operator()(const LDSRecord& record) const
            {
                return !record.IsValid();
            }
        };

        struct RecordSort
        {
            constexpr bool operator()(const LDSRecord& a, const LDSRecord& b) const
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
            constexpr bool operator()(const LDSRecord& record, hash64_t id) const
            {
                return (record.GetId() & Mask) < id;
            }
        };

        struct UnsortedMaskedRecordIdPred
        {
            hash64_t RecordId;
            hash64_t Mask;
            constexpr bool operator()(const LDSRecord& record) const
            {
                return (record.GetId() & Mask) == RecordId;
            }
        };

        // Find the index of a record by searching the sorted range followed by the unsorted range.
        uint32 FindIndexOfRecord(hash64_t recordId, hash64_t mask, const LDSObjectRun& run) const
        {
            if (Records.IsEmpty())
            {
                return Index<uint32>::None;
            }

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
        uint32 FindIndexOfRecordInSortedRange(hash64_t recordId, hash64_t mask, const LDSObjectRun& run) const
        {
            if (SortedNum == 0)
            {
                return Index<uint32>::None;
            }

            // If a valid run was specified, try to use that because it should greatly reduce the search area.
            if (run.Start < run.End && run.End <= SortedNum)
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
                LDSObjectRun afterRun = { run.End, SortedNum };
                return FindIndexOfRecordInSortedRun(recordId, mask, afterRun);
            }

            // If no run was specified then just search the entire sorted range.
            return FindIndexOfRecordInSortedRange(recordId, mask);
        }

        // Search a subset of the sorted range. This operation is O(log N).
        uint32 FindIndexOfRecordInSortedRun(hash64_t recordId, hash64_t mask, const LDSObjectRun& run) const
        {
            if (run.Start >= run.End || run.End >= SortedNum)
            {
                // Invalid range.
                return Index<uint32>::None;
            }
            auto begin = Records.begin() + run.Start;
            auto end = Records.begin() + run.End;
            return FindIndexOfRecordInSortedRange(begin, end, recordId, mask);
        }

        // Search the entire sorted range. This operation is O(log N).
        uint32 FindIndexOfRecordInSortedRange(hash64_t recordId, hash64_t mask) const
        {
            uint32 sortedNum = SortedNum;
            if (sortedNum == 0)
            {
                // Not sorted yet so there's nothing to search.
                return Index<uint32>::None;
            }
            auto begin = Records.begin();
            auto end = begin + sortedNum;
            return FindIndexOfRecordInSortedRange(begin, end, recordId, mask);
        }

        // Search a subset of the sorted range. This operation is O(log N).
        uint32 FindIndexOfRecordInSortedRange(
            const auto& begin,
            const auto& end,
            hash64_t recordId,
            hash64_t mask) const
        {
            if (begin == end)
            {
                return Index<uint32>::None;
            }

            auto iter = std::lower_bound(
                begin,
                end,
                recordId & mask,
                SortedMaskedRecordIdPred{ mask });

            // We did not find the record within the sorted range.
            if (iter == end || (iter->GetId() & mask) != (recordId & mask))
            {
                return Index<uint32>::None;
            }

            // We found the record within the sorted range. Return the absolute index.
            return (uint32)(iter - Records.begin());
        }

        // Search the entire unsorted range. This operation is O(N).
        uint32 FindIndexOfRecordInUnsortedRange(hash64_t recordId, hash64_t mask) const
        {
            auto iter = std::find_if(
                Records.begin() + SortedNum,
                Records.end(),
                UnsortedMaskedRecordIdPred{recordId & mask, mask});

            // We did not find the record within the unsorted range.
            if (iter == Records.end() || (iter->GetId() & mask) != (recordId & mask))
            {
                return Index<uint32>::None;
            }

            // We found the record within the unsorted range. Return the absolute index.
            return (uint32)(iter - Records.begin());
        }
        
        TContainer Records;
        uint32 SortedNum = 0;
    };

    template <size_t N>
    using TFixedRecordStore = TLDSRecordStore<TFixedArray<LDSRecord, N>>;

    using RecordStore = TLDSRecordStore<TArray2<LDSRecord>>;
}
