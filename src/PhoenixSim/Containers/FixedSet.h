#pragma once

#include "FixedMemory.h"

namespace Phoenix
{
    template <class TKey, class TStoragePolicy>
    class TSetBase
    {
    protected:
        using TStorage = TContiguousStorage<TKey, TStoragePolicy>;
        TStorage Storage;
        uint32 Size = 0;
    };

    template <class TKey>
    class TSetBase<TKey, FixedStoragePolicy>
    {
    public:

        TSetBase() = default;

        template <class TAllocator>
        TSetBase(TAllocator& allocator, uint32 capacity)
            : Storage(allocator, capacity)
        {
        }

        template <class TAllocator, class TOtherStoragePolicy>
        TSetBase(TAllocator& allocator, uint32 capacity, const TSetBase<TKey, TOtherStoragePolicy>& other)
            : Storage(allocator, capacity, other.Storage)
            , Size(other.Size)
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
        using TStorage = TFixedStorage<TKey>;
        TStorage Storage;
        uint32 Size = 0;
    };

    template <class TKey, class TStoragePolicy, class THasher = std::hash<TKey>>
    class TSet : TSetBase<TKey, TStoragePolicy>
    {
        using Super = TSetBase<TKey, TStoragePolicy>;
        using Super::Super;
        using Super::Storage;
        using Super::Size;

    public:

        PHX_FORCEINLINE uint32 GetCapacity() const
        {
            return Storage.GetCapacity();
        }

        PHX_FORCEINLINE bool IsFull() const
        {
            return Size == Storage.GetCapacity();
        }

        PHX_FORCEINLINE bool IsEmpty() const
        {
            return Size == 0;
        }

        PHX_FORCEINLINE size_t GetNum() const
        {
            return Size;
        }

        void Reset()
        {
            Storage.SetZero();
            Size = 0;
        }

        bool Insert(const TKey& key)
        {
            PHX_ASSERT(key != 0);

            size_t index = FindSlot(key);
            if (Storage[index] != 0)
            {
                // Already in set
                if (Storage[index] == key)
                    return true;

                // Set is full
                return false;
            }

            Storage[index] = key;
            ++Size;

            return true;
        }

        bool Contains(const TKey& key) const
        {
            size_t index = FindSlot(key);
            return Storage[index] == key; 
        }

        size_t FindSlot(const TKey& key) const
        {
            size_t hash = Hash(key);
            size_t index = hash % GetCapacity();
            size_t startIndex = index;
            while (Storage[index] != 0 && Storage[index] != key)
            {
                index = (index + 1) % GetCapacity();
                if (index == startIndex)
                    break;
            }
            return index;
        }

    private:

        PHX_FORCEINLINE static size_t Hash(const TKey& key)
        {
            static const THasher hasher;
            return hasher(key);
        }
    };

    template <class TKey, uint32 N, class THasher = std::hash<TKey>>
    using TInlineSet = TSet<TKey, InlineStoragePolicy<N>, THasher>;

    template <class TKey, class THasher = std::hash<TKey>>
    using TFixedSet = TSet<TKey, FixedStoragePolicy, THasher>;

    template <class TKey, class THasher = std::hash<TKey>>
    using THeapSet = TSet<TKey, HeapStoragePolicy, THasher>;
}
