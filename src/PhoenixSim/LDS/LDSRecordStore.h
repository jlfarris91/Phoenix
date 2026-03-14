
#pragma once

#include "LDSRecord.h"
#include "PhoenixSim/Containers/FixedSortedList.h"

namespace Phoenix::LDS
{
    template <class TStoragePolicy>
    class TLDSRecordStoreBase
    {
    protected:
        using TStorage = TSortedList<LDSRecord, LDSRecord::GetItemKey, TStoragePolicy>;
        TStorage Storage;
    };

    template <>
    class TLDSRecordStoreBase<FixedStoragePolicy>
    {
    public:
        TLDSRecordStoreBase() = default;

        template <class TAllocator>
        TLDSRecordStoreBase(TAllocator& allocator, uint32 capacity)
            : Storage(allocator, capacity)
        {
        }

        template <class TAllocator, class TOtherStoragePolicy>
        TLDSRecordStoreBase(TAllocator& allocator, uint32 capacity, const TLDSRecordStoreBase<TOtherStoragePolicy>& other)
            : Storage(allocator, capacity, other.Storage)
        {
        }

        PHX_FORCEINLINE static uint32 GetAllocSizeBytes(uint32 capacity)
        {
            return TStorage::GetAllocSizeBytes(capacity);
        }

        PHX_FORCEINLINE uint32 GetAllocSizeBytes() const
        {
            return Storage.GetAllocSizeBytes();
        }

    protected:
        using TStorage = TFixedSortedList<LDSRecord, LDSRecord::GetItemKey>;
        TStorage Storage;
    };

    template <class TStoragePolicy>
    class PHOENIX_SIM_API TLDSRecordStore : public TLDSRecordStoreBase<TStoragePolicy>
    {
        using Super = TLDSRecordStoreBase<TStoragePolicy>;
        using Super::Super;
        using Super::Storage;

    public:

        static constexpr hash64_t ObjectMask = 0xFFFFFFFF00000000ULL;
        static constexpr hash64_t PropertyMask = 0x00000000FFFFFFFFULL;

        PHX_FORCEINLINE const LDSRecord* GetData() const
        {
            return Storage.GetData();
        }

        PHX_FORCEINLINE uint32 GetCapacity() const
        {
            return Storage.GetCapacity();
        }

        PHX_FORCEINLINE uint32 GetNum() const
        {
            return Storage.GetNum();
        }

        PHX_FORCEINLINE bool IsFull() const
        {
            return Storage.IsFull();
        }

        PHX_FORCEINLINE bool IsEmpty() const
        {
            return Storage.IsEmpty();
        }

        PHX_FORCEINLINE void Reset()
        {
            Storage.Reset();
        }

        PHX_FORCEINLINE void Sort()
        {
            Storage.Sort();
        }

        const LDSRecord* FindRecord(hash64_t recordId, hash64_t mask = -1) const
        {
            uint32 index;
            return Storage.GetFirstItem(recordId, index, MaskedKeyEquals { mask });
        }

        const LDSRecord* FindRecord(const FName& objectId, const FName& propertyId) const
        {
            return FindRecord(ToRecordId(objectId, propertyId));
        }

        bool HasObject(const FName& objectId) const
        {
            return FindRecord(ToRecordId(objectId), ObjectMask) != nullptr;
        }

        bool HasRecord(const FName& objectId, const FName& propertyId) const
        {
            return FindRecord(ToRecordId(objectId, propertyId)) != nullptr;
        }

        template <class TCallback>
        void ForEachRecord(const FName& objectId, const TCallback& callback) const
        {
            Storage.ForEachItem(ToRecordId(objectId), callback, MaskedKeyEquals { ObjectMask });
        }

        ELDSValueType GetTypeRecordValueType(const FName& objectId, const FName& propertyId) const
        {
            const LDSRecord* record = FindRecord(objectId, propertyId);
            return record ? record->GetValueType() : ELDSValueType::Unknown;
        }

        template <class T>
        const T& GetRecordValueAs(const FName& objectId, const FName& propertyId, const T& defaultValue = {})
        {
            const LDSRecord* record = FindRecord(objectId, propertyId);
            return record ? record->GetValueAs<T>() : defaultValue;
        }

        bool SetRecord(const FName& objectId, const FName& propertyId, const LDSTypedValue& value)
        {
            if (LDSRecord* record = Storage.GetItem(ToRecordId(objectId, propertyId)))
            {
                record->SetValue(value);
                return true;
            }
            return Storage.EmplaceBack(objectId, propertyId, value);
        }

        bool SetRecord(const FName& objectId, const FName& propertyId, const LDSValue& value, ELDSValueType type)
        {
            return SetRecord(objectId, propertyId, LDSTypedValue(value, type));
        }

        PHX_FORCEINLINE static hash64_t ToRecordId(const FName& objectId, const FName& propertyId = FName::None)
        {
            return (uint64)(uint32)objectId << 32 | (uint32)propertyId;
        }

    private:

        struct MaskedKeyEquals
        {
            hash64_t Mask;
            bool operator()(hash64_t a, hash64_t b) const
            {
                return (a & Mask) == (b & Mask);
            }
        };
    };

    using FixedLDSRecordStore = TLDSRecordStore<FixedStoragePolicy>;
    using HeapLDSRecordStore = TLDSRecordStore<HeapStoragePolicy>;
}
