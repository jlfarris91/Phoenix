#pragma once

#include "PhoenixSim/OffsetRef.h"
#include "PhoenixSim/Platform.h"

#include <array>

namespace Phoenix
{
    template <class TStorage>
    concept StorageConcept = requires (TStorage& s, const TStorage& cs)
    {
        { TStorage::CanGrow() } -> std::same_as<bool>;
        { cs.GetCapacity() } -> std::same_as<uint32>;
        { cs.IsValidIndex(uint32{}) } -> std::same_as<bool>;
        { s.GetData() } -> std::same_as<typename TStorage::TElement*>;
        { cs.GetData() } -> std::same_as<const typename TStorage::TElement*>;
        { s.operator[](uint32{}) } -> std::same_as<typename TStorage::TElement&>;
        { cs.operator[](uint32{}) } -> std::same_as<const typename TStorage::TElement&>;
    };

    static constexpr uint32 NoConstTimeCapacity = static_cast<uint32>(-1);

    template <uint32 N>
    struct StoragePolicy { static constexpr uint32 Capacity = N; };

    template <uint32 N> struct InlineStoragePolicy : StoragePolicy<N> {};
    struct FixedStoragePolicy : StoragePolicy<NoConstTimeCapacity> {};
    struct HeapStoragePolicy : StoragePolicy<NoConstTimeCapacity> {};

    template <class, class>
    class TContiguousStorage;

    template <class T, uint32 N>
    class TContiguousStorage<T, InlineStoragePolicy<N>>
    {
        static_assert(N > 0);

    public:
        using TElement = T;

        TContiguousStorage()
        {
        }

        PHX_FORCEINLINE static bool CanGrow()
        {
            return false;
        }

        PHX_FORCEINLINE static uint32 GetCapacity()
        {
            return N;
        }

        PHX_FORCEINLINE static bool IsValidIndex(uint32 index)
        {
            return index < N;
        }

        PHX_FORCEINLINE T* GetData()
        {
            return Data.data();
        }

        PHX_FORCEINLINE const T* GetData() const
        {
            return Data.data();
        }

        PHX_FORCEINLINE T& operator[](uint32 index)
        {
            return Data[index];
        }

        PHX_FORCEINLINE const T& operator[](uint32 index) const
        {
            return Data[index];
        }

        PHX_FORCEINLINE void SetZero()
        {
            Data.fill({});
        }

        PHX_FORCEINLINE auto begin()
        {
            return Data.begin();
        }

        PHX_FORCEINLINE auto begin() const
        {
            return Data.begin();
        }

        PHX_FORCEINLINE auto end()
        {
            return Data.end();
        }

        PHX_FORCEINLINE auto end() const
        {
            return Data.end();
        }

    protected:

        std::array<T, N> Data;
    };

    template <class T, uint32 N>
    using TInlineStorage = TContiguousStorage<T, InlineStoragePolicy<N>>;

    static_assert(StorageConcept<TInlineStorage<uint32, 1>> == true);

    template <class T>
    class TContiguousStorage<T, FixedStoragePolicy>
    {
    public:

        using TElement = T;

        TContiguousStorage() = default;

        TContiguousStorage(uint32 offset, uint32 capacity)
            : Capacity(capacity)
            , Data(offset)
        {
            SetZero();
        }

        TContiguousStorage(T* ptr, uint32 capacity)
            : Capacity(capacity)
            , Data(ptr)
        {
            SetZero();
        }

        template <class TAllocator>
        TContiguousStorage(TAllocator& allocator, uint32 capacity)
            : Capacity(capacity)
            , Data(allocator.template Allocate<T>(capacity))
        {
            SetZero();
        }

        template <class TAllocator>
        TContiguousStorage(TAllocator& allocator, uint32 capacity, const TContiguousStorage& other)
            : TContiguousStorage(allocator, capacity)
        {
            SetZero();

            uint32 minSize = std::min(capacity, other.GetCapacity());
            memcpy(GetData(), other.GetData(), minSize);
        }

        PHX_FORCEINLINE static bool CanGrow()
        {
            return false;
        }

        PHX_FORCEINLINE uint32 GetCapacity() const
        {
            return Capacity;
        }

        PHX_FORCEINLINE bool IsValidIndex(uint32 index) const
        {
            return index < Capacity;
        }

        PHX_FORCEINLINE T* GetData()
        {
            return Data.Get();
        }

        PHX_FORCEINLINE const T* GetData() const
        {
            return Data.Get();
        }

        PHX_FORCEINLINE T& operator[](uint32 index)
        {
            return *(Data.Get() + index);
        }

        PHX_FORCEINLINE const T& operator[](uint32 index) const
        {
            return *(Data.Get() + index);
        }

        PHX_FORCEINLINE static uint32 GetAllocSizeBytes(uint32 capacity)
        {
            return sizeof(T) * capacity;
        }

        PHX_FORCEINLINE uint32 GetAllocSizeBytes() const
        {
            return GetAllocSizeBytes(Capacity);
        }

        PHX_FORCEINLINE void SetZero()
        {
            T* item = begin();
            for (; item != end(); ++item)
            {
                new (item) T();
            }
        }

        PHX_FORCEINLINE T* begin()
        {
            return Data.Get();
        }

        PHX_FORCEINLINE const T* begin() const
        {
            return Data.Get();
        }

        PHX_FORCEINLINE T* end()
        {
            return Data.Get() + Capacity;
        }

        PHX_FORCEINLINE const T* end() const
        {
            return Data.Get() + Capacity;
        }

    protected:

        uint32 Capacity = 0;
        TSelfOffsetRef<T> Data;
    };

    template <class T>
    using TFixedStorage = TContiguousStorage<T, FixedStoragePolicy>;

    static_assert(StorageConcept<TFixedStorage<uint32>> == true);

    template <class T>
    class TContiguousStorage<T, HeapStoragePolicy>
    {
    public:

        using TElement = T;

        TContiguousStorage() = default;

        TContiguousStorage(uint32 count)
            : Data(count)
        {
        }

        PHX_FORCEINLINE static bool CanGrow()
        {
            return true;
        }

        PHX_FORCEINLINE uint32 GetCapacity() const
        {
            return static_cast<uint32>(Data.capacity());
        }

        PHX_FORCEINLINE bool IsValidIndex(uint32 index) const
        {
            return index < Data.size();
        }

        PHX_FORCEINLINE T* GetData()
        {
            return Data.data();
        }

        PHX_FORCEINLINE const T* GetData() const
        {
            return Data.data();
        }

        PHX_FORCEINLINE T& operator[](uint32 index)
        {
            return Data[index];
        }

        PHX_FORCEINLINE const T& operator[](uint32 index) const
        {
            return Data[index];
        }

        PHX_FORCEINLINE uint32 GetSize() const
        {
            return Data.size();
        }

        PHX_FORCEINLINE auto begin()
        {
            return Data.begin();
        }

        PHX_FORCEINLINE auto begin() const
        {
            return Data.begin();
        }

        PHX_FORCEINLINE auto end()
        {
            return Data.end();
        }

        PHX_FORCEINLINE auto end() const
        {
            return Data.end();
        }

        PHX_FORCEINLINE void SetZero()
        {
            Data.clear();
        }

    protected:

        std::vector<T> Data;
    };

    template <class T>
    using THeapStorage = TContiguousStorage<T, HeapStoragePolicy>;

    static_assert(StorageConcept<THeapStorage<uint32>> == true);
}
