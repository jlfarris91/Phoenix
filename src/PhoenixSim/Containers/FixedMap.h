#pragma once

#include "PhoenixSim/Containers/FixedMemory.h"

namespace Phoenix
{
    template <class TKey, class TValue>
    struct TFixedMapElement
    {
        TKey Key;
        TValue Value;
    };

    template <class TKey, class TValue, class TStoragePolicy>
    class TMapBase
    {
    protected:
        using TItem = TFixedMapElement<TKey, TValue>;
        using TStorage = TContiguousStorage<TItem, TStoragePolicy>;
        TStorage Storage;
        uint32 Size = 0;
    };

    template <class TKey, class TValue>
    class TMapBase<TKey, TValue, FixedStoragePolicy>
    {
    public:

        TMapBase() = default;

        template <class TAllocator>
        TMapBase(TAllocator& allocator, uint32 capacity)
            : Storage(allocator, capacity)
        {
        }

        template <class TAllocator, class TOtherStoragePolicy>
        TMapBase(TAllocator& allocator, uint32 capacity, const TMapBase<TKey, TValue, TOtherStoragePolicy>& other)
            : Storage(allocator, capacity)
        {
            // for (const TItem& element : other)
            // {
            //     if (element.Key != TKey{})
            //     {
            //         Insert(element);
            //     }
            //     if (Storage.IsFull())
            //     {
            //         break;
            //     }
            // }
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
        using TItem = TFixedMapElement<TKey, TValue>;
        using TStorage = TFixedStorage<TItem>;
        TStorage Storage;
        uint32 Size = 0;
    };

    template <class TKey, class TValue, class TStoragePolicy, class THasher = std::hash<TKey>>
    class TMap : public TMapBase<TKey, TValue, TStoragePolicy>
    {
        using Super = TMapBase<TKey, TValue, TStoragePolicy>;
        using Super::Super;
        using Super::Storage;
        using Super::Size;

        using TItem = typename Super::TItem;

    public:

        PHX_FORCEINLINE uint32 GetCapacity() const
        {
            return Storage.GetCapacity();
        }

        PHX_FORCEINLINE TItem* GetData()
        {
            return Storage.GetData();
        }

        PHX_FORCEINLINE const TItem* GetData() const
        {
            return Storage.GetData();
        }

        uint32 GetNum() const
        {
            return Size;
        }

        bool IsFull() const
        {
            return Size == GetCapacity();
        }

        bool IsEmpty() const
        {
            return Size == 0;
        }

        void Reset()
        {
            Storage.SetZero();
            Size = 0;
        }

        bool Insert(const TKey& key, const TValue& value = {})
        {
            TItem* data = GetData();
            size_t slot = FindSlot(key);

            if (data[slot].Key != TKey{})
            {
                // KVP already exists
                if (data[slot].Key == key)
                {
                    data[slot].Value = value;
                    return true;
                }

                // Map is full
                return false;
            }

            data[slot].Key = key;
            data[slot].Value = value;
            ++Size;

            return true;
        }

        template <class ...TArgs>
        bool Emplace(const TKey& key, const TArgs&... args)
        {
            TItem* data = GetData();
            size_t slot = FindSlot(key);

            if (data[slot].Key != TKey{})
            {
                // KVP already exists
                if (data[slot].Key == key)
                {
                    new (&data[slot].Value) TValue(args...);
                    return true;
                }

                // Map is full
                return false;
            }

            data[slot].Key = key;
            new (&data[slot].Value) TValue(args...);
            ++Size;

            return true;
        }

        TValue& InsertDefaulted_GetRef(const TKey& key)
        {
            TItem* data = GetData();
            size_t slot = FindSlot(key);

            if (data[slot].Key != TKey{})
            {
                // KVP already exists
                if (data[slot].Key == key)
                {
                    data[slot].Value = TValue();
                    return data[slot].Value;
                }

                // Map is full
                PHX_ASSERT(false);
            }

            data[slot].Key = key;
            data[slot].Value = TValue();
            ++Size;

            return data[slot].Value;
        }

        bool Contains(const TKey& key) const
        {
            size_t slot = FindSlot(key);
            return GetData()[slot].Key == key;
        }

        TValue* GetPtr(const TKey& key)
        {
            size_t slot = FindSlot(key);
            if (GetData()[slot].Key == key)
            {
                return &GetData()[slot].Value;
            }
            return nullptr;
        }

        const TValue* GetPtr(const TKey& key) const
        {
            size_t slot = FindSlot(key);
            if (GetData()[slot].Key == key)
            {
                return &GetData()[slot].Value;
            }
            return nullptr;
        }

        TValue& Get(const TKey& key)
        {
            return *GetPtr(key);
        }

        const TValue& Get(const TKey& key) const
        {
            return *GetPtr(key);
        }

        TValue& operator[](const TKey& key)
        {
            return Get(key);
        }

        const TValue& operator[](const TKey& key) const
        {
            return Get(key);
        }

        TValue* FindOrAdd(const TKey& key, const TValue& value)
        {
            TItem* data = GetData();
            size_t slot = FindSlot(key);

            if (data[slot].Key != TKey{})
            {
                // KVP already exists
                if (data[slot].Key == key)
                {
                    return &data[slot].Value;
                }

                // Map is full
                return nullptr;
            }

            data[slot].Key = key;
            data[slot].Value = value;
            ++Size;

            return &data[slot].Value;
        }

        TValue* FindOrAddDefaulted(const TKey& key)
        {
            TItem* data = GetData();
            size_t slot = FindSlot(key);

            if (data[slot].Key != TKey{})
            {
                // KVP already exists
                if (data[slot].Key == key)
                {
                    return &data[slot].Value;
                }

                // Map is full
                return nullptr;
            }

            data[slot].Key = key;
            data[slot].Value = TValue();
            ++Size;

            return &data[slot].Value;
        }

        // See https://en.wikipedia.org/wiki/Open_addressing
        bool Remove(const TKey& key)
        {
            TItem* data = GetData();
            size_t i = FindSlot(key);

            // Slot is not occupied
            if (data[i].Key != key)
            {
                return false;
            }

            // Mark slot as unoccupied
            data[i].Key = TKey{};

            PHX_ASSERT(!IsEmpty());
            --Size;

            // Attempt to fill item
            size_t j = i;
            for (;;)
            {
                j = (j + 1) % GetCapacity();
                if (data[j].Key == TKey{})
                {
                    break;
                }

                size_t k = Hash(data[j].Key, GetCapacity());

                // determine if k lies cyclically in (i,j]
                // i ≤ j: |    i..k..j    |
                // i > j: |.k..j     i....| or |....j     i..k.|
                if (i <= j)
                {
                    if (i < k && k <= j)
                    {
                        continue;
                    }
                }
                else if (k <= j || i < k)
                {
                    continue;
                }

                // Move slot[j] into slot[i]
                data[i] = data[j];

                // Mark slot[j] as unoccupied
                data[j].Key = TKey{};

                i = j;
            }

            return true;
        }

        struct Iter
        {
            Iter(TMap* map, size_t index) : Map(map), Index(index) {}

            std::pair<TKey, TValue&> operator*() const
            {
                const TItem* data = Map->GetData();
                return { data[Index].Key, data[Index].Value };
            }

            Iter& operator++()
            {
                Index = Map->FindNextOccupiedSlot(Index + 1);
                return *this;
            }

            bool operator==(const Iter& other) const = default;

            TMap* Map;
            size_t Index;
        };

        Iter begin() { return Iter(this, FindNextOccupiedSlot(0)); }
        Iter end() { return Iter(this, GetCapacity()); }

        struct ConstIter
        {
            ConstIter(const TMap* map, size_t index) : Map(map), Index(index) {}

            const TItem& operator*() const
            {
                return Map->GetData()[Index];
            }

            ConstIter& operator++()
            {
                Index = Map->FindNextOccupiedSlot(Index + 1);
                return *this;
            }

            bool operator==(const ConstIter& other) const = default;

            const TMap* Map;
            size_t Index;
        };

        ConstIter begin() const { return ConstIter(this, FindNextOccupiedSlot(0)); }
        ConstIter end() const { return ConstIter(this, GetCapacity()); }

    private:

        static size_t Hash(const TKey& key, uint32 capacity)
        {
            static THasher hasher;
            return hasher(key) % capacity;
        }

        size_t FindSlot(const TKey& key) const
        {
            const TItem* data = GetData();
            size_t hash = Hash(key, GetCapacity());
            size_t index = hash % GetCapacity();
            size_t startIndex = index;
            while (data[index].Key != TKey{} && data[index].Key != key)
            {
                index = (index + 1) % GetCapacity();
                if (index == startIndex)
                    break;
            }
            return index;
        }

        size_t FindNextOccupiedSlot(size_t index) const
        {
            const TItem* data = GetData();
            while (index < GetCapacity() && data[index].Key == TKey{})
            {
                ++index;
            }
            return index;
        }
    };

    template <class TKey, class TValue, uint32 N, class THasher = std::hash<TKey>>
    using TInlineMap = TMap<TKey, TValue, InlineStoragePolicy<N>, THasher>;

    template <class TKey, class TValue, class THasher = std::hash<TKey>>
    using TFixedMap = TMap<TKey, TValue, FixedStoragePolicy, THasher>;

    template <class TKey, class TValue, class THasher = std::hash<TKey>>
    using THeapMap = TMap<TKey, TValue, HeapStoragePolicy, THasher>;
}
