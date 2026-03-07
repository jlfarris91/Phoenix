
#pragma once

#include "PhoenixSim/Containers/FixedMemory.h"

namespace Phoenix
{
    template <class T, class TStoragePolicy>
    class TQueueBase
    {
    protected:
        using TStorage = TContiguousStorage<T, TStoragePolicy>;
        TStorage Storage;
        uint32 Start = 0, End = 0;
    };

    template <class T>
    class TQueueBase<T, FixedStoragePolicy>
    {
    public:

        TQueueBase() = default;

        template <class TAllocator>
        TQueueBase(TAllocator& allocator, uint32 capacity)
            : Storage(allocator, capacity)
        {
        }

        template <class TAllocator, class TOtherStoragePolicy>
        TQueueBase(TAllocator& allocator, uint32 capacity, const TQueueBase<T, TOtherStoragePolicy>& other)
            : Storage(allocator, capacity, other.Storage)
            , Start(other.Start)
            , End(other.End)
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
        using TStorage = TFixedStorage<T>;
        TStorage Storage;
        uint32 Start = 0, End = 0;
    };

    template <class T, class TStoragePolicy, bool AllowOverwrite = false>
    class TQueue : TQueueBase<T, TStoragePolicy>
    {
        using Super = TQueueBase<T, TStoragePolicy>;
        using Super::Super;
        using Super::Storage;
        using Super::Start;
        using Super::End;

    public:

        PHX_FORCEINLINE uint32 GetCapacity() const
        {
            return Storage.GetCapacity();
        }

        bool IsEmpty() const
        {
            return Start == End;
        }

        bool IsFull() const
        {
            return GetNum() == GetCapacity() - 1;
        }

        uint32 GetNum() const
        {
            if (Start > End)
                return End + GetCapacity() - Start;
            return End - Start;
        }

        bool Contains(const T& value)
        {
            for (uint32 i = Start; i != End; i = MoveIndex(i, 1))
            {
                if (Storage[i] == value)
                    return true;
            }
            return false;
        }

        void Enqueue(const T& value)
        {
            PHX_ASSERT(AllowOverwrite || !IsFull());
            Storage[End] = value;
            if (AllowOverwrite && IsFull())
            {
                Start = MoveIndex(Start, 1);
            }
            End = MoveIndex(End, 1);
        }

        void EnqueueUnique(const T& value)
        {
            if (Contains(value))
                return;
            Enqueue(value);
        }

        T Dequeue()
        {
            PHX_ASSERT(!IsEmpty());
            T value = Storage[Start];
            Start = MoveIndex(Start, 1);
            return value;
        }

        void Reset()
        {
            Storage.SetZero();
            Start = 0;
            End = 0;
        }

        static constexpr uint32 MoveIndex(const uint32 i, const uint32 capacity, int32 n)
        {
            int32 r = static_cast<int32>(i) + n;
            if (r < 0) return static_cast<int32>(capacity) + r;
            return r % static_cast<int32>(capacity);
        }

        uint32 MoveIndex(const uint32 i, int32 n) const
        {
            return MoveIndex(i, GetCapacity(), n);
        }

        T& operator[](uint32 n)
        {
            return Storage[MoveIndex(Start, n)];
        }

        const T& operator[](uint32 n) const
        {
            return Storage[MoveIndex(Start, n)];
        }

        struct Iter
        {
            using value_type = T;
            using element_type = T;
            using iterator_category = std::contiguous_iterator_tag;

            Iter(T* data, uint32 capacity, uint32 index)
                : DataPtr(data)
                , Capacity(capacity)
                , Index(index)
            {
            }

            T* operator->() const
            {
                return DataPtr + Index;
            }

            T& operator*() const
            {
                return *(DataPtr + Index);
            }

            T& operator[](int64 n) const
            {
                return *(DataPtr + MoveIndex(Index, Capacity, n));
            }

            Iter& operator++()
            {
                Index = MoveIndex(Index, Capacity, 1);
                return *this;
            }

            Iter operator++(int32) const
            {
                auto prev = *this;
                ++*this;
                return prev;
            }

            Iter& operator--()
            {
                Index = MoveIndex(Index, Capacity, -1);
                return *this;
            }

            Iter operator--(int32) const
            {
                auto prev = *this;
                --*this;
                return prev;
            }

            Iter& operator+=(int64 n)
            {
                Index = MoveIndex(Index, Capacity, static_cast<int32>(n));
                return *this;
            }

            Iter operator+(int64 n)
            {
                auto next = *this;
                next += n;
                return next;
            }

            Iter& operator-=(int64 n)
            {
                Index = MoveIndex(Index, Capacity, static_cast<int32>(-n));
                return *this;
            }

            Iter operator-(int64 n)
            {
                auto next = *this;
                next -= n;
                return next;
            }

            int64 operator-(const Iter& other) const
            {
                return static_cast<int64>(Index) - static_cast<int64>(other.Index);
            }

            bool operator==(const Iter& other) const
            {
                return DataPtr == other.DataPtr && Capacity == other.Capacity && Index == other.Index;
            }

            friend auto operator<=>(const Iter&, const Iter&) = default;

        private:
            T* DataPtr;
            uint32 Capacity;
            uint32 Index;
        };

        Iter begin()
        {
            return Iter(Storage.GetData(), GetCapacity(), Start);
        }

        Iter end()
        {
            return Iter(Storage.GetData(), GetCapacity(), End);
        }

        struct ConstIter
        {
            using value_type = T;
            using element_type = T;
            using iterator_category = std::contiguous_iterator_tag;

            ConstIter(const T* data, uint32 capacity, uint32 index)
                : DataPtr(data)
                , Capacity(capacity)
                , Index(index)
            {
            }

            const T* operator->() const
            {
                return DataPtr + Index;
            }

            const T& operator*() const
            {
                return *(DataPtr + Index);
            }

            const T& operator[](int32 n) const
            {
                return *(DataPtr + MoveIndex(Index, Capacity, n));
            }

            ConstIter& operator++()
            {
                Index = MoveIndex(Index, Capacity, 1);
                return *this;
            }

            ConstIter operator++(int32 n) const
            {
                return { DataPtr, MoveIndex(Index, Capacity, n) };
            }

            ConstIter& operator--()
            {
                Index = MoveIndex(Index, Capacity, -1);
                return *this;
            }

            ConstIter operator--(int32 n) const
            {
                return { DataPtr, MoveIndex(Index, Capacity, -n) };
            }

            ConstIter& operator+=(int32 n)
            {
                Index = MoveIndex(Index, Capacity, n);
                return *this;
            }

            ConstIter& operator-=(int32 n)
            {
                Index = MoveIndex(Index, Capacity, -n);
                return *this;
            }

            friend auto operator<=>(ConstIter, ConstIter) = default;

            friend long operator-(const ConstIter& a, const ConstIter& b)
            {
                if (a.Index > b.Index) return a.Index - b.Index;
                return a.Index + a.Capacity - b.Index;
            }

            friend ConstIter operator+(ConstIter i, int32 n)
            {
                return { i.DataPtr, MoveIndex(i.Index, i.Capacity, n) };
            }

            friend ConstIter operator-(ConstIter i, int32 n)
            {
                return { i.DataPtr, MoveIndex(i.Index, i.Capacity, -n) };
            }

            friend ConstIter operator+(int32 n, ConstIter i)
            {
                return { i.DataPtr, MoveIndex(i.Index, i.Capacity, n) };
            }

            bool operator==(const ConstIter& other) const
            {
                return DataPtr == other.DataPtr && Capacity == other.Capacity && Index == other.Index;
            }

        private:
            const T* DataPtr;
            uint32 Capacity;
            uint32 Index;
        };

        ConstIter begin() const
        {
            return ConstIter(Storage.GetData(), GetCapacity(), Start);
        }

        ConstIter end() const
        {
            return ConstIter(Storage.GetData(), GetCapacity(), End);
        }
    };

    template <class T, uint32 N, bool AllowOverwrite = false>
    using TInlineQueue = TQueue<T, InlineStoragePolicy<N>, AllowOverwrite>;

    template <class T, bool AllowOverwrite = false>
    using TFixedQueue = TQueue<T, FixedStoragePolicy, AllowOverwrite>;

    template <class T, bool AllowOverwrite = false>
    using THeapQueue = TQueue<T, HeapStoragePolicy, AllowOverwrite>;

    static_assert(TInlineQueue<int, 32>::MoveIndex(0, 32, 32) == 0);
    static_assert(TInlineQueue<int, 32>::MoveIndex(0, 32, 31) == 31);
    static_assert(TInlineQueue<int, 32>::MoveIndex(0, 32, 30) == 30);
    static_assert(TInlineQueue<int, 32>::MoveIndex(0, 32, 3) == 3);
    static_assert(TInlineQueue<int, 32>::MoveIndex(0, 32, 2) == 2);
    static_assert(TInlineQueue<int, 32>::MoveIndex(0, 32, 1) == 1);
    static_assert(TInlineQueue<int, 32>::MoveIndex(0, 32, 0) == 0);
    static_assert(TInlineQueue<int, 32>::MoveIndex(0, 32, -1) == 31);
    static_assert(TInlineQueue<int, 32>::MoveIndex(0, 32, -2) == 30);
    static_assert(TInlineQueue<int, 32>::MoveIndex(0, 32, -3) == 29);
    static_assert(TInlineQueue<int, 32>::MoveIndex(0, 32, -32) == 0);
    static_assert(TInlineQueue<int, 32>::MoveIndex(0, 32, -31) == 1);
    static_assert(TInlineQueue<int, 32>::MoveIndex(0, 32, -30) == 2);
}