#pragma once

#include <cassert>

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Containers/FixedMemory.h"

namespace Phoenix
{
    template <class T, class TStoragePolicy>
    class TArrayBase
    {
    protected:
        TContiguousStorage<T, TStoragePolicy> Storage;
        uint32 Size = 0;
    };

    template <class T>
    class TArrayBase<T, FixedStoragePolicy>
    {
        using TStorage = TFixedStorage<T>;

    public:

        PHX_DECLARE_BLOCK_CONTAINER(TArrayBase)
        {
            uint32 Capacity;
        };

    protected:
        TStorage Storage;
        uint32 Size = 0;
    };

    template <class T>
    void TArrayBase<T, FixedStoragePolicy>::Construct(BlockBufferAllocator& allocator, const Config& config)
    {
        Storage.Construct(allocator, config.Capacity);
    }

    template <class T>
    BlockBufferLayout TArrayBase<T, FixedStoragePolicy>::StaticLayout(const Config& config)
    {
        return BlockBufferLayout::For<TArrayBase>().Container<TStorage>(config.Capacity);
    }

    template <class T, class TStoragePolicy>
    class TArray : public TArrayBase<T, TStoragePolicy>
    {
        using Super = TArrayBase<T, TStoragePolicy>;
        using Super::Super;
        using Super::Storage;
        using Super::Size;

    public:

        using TItem = T;

        PHX_FORCEINLINE T& operator[](uint32 index)
        {
            return Storage.operator[](index);
        }

        PHX_FORCEINLINE const T& operator[](uint32 index) const
        {
            return Storage.operator[](index);
        }

        PHX_FORCEINLINE T* GetData()
        {
            return Storage.GetData();
        }

        PHX_FORCEINLINE const T* GetData() const
        {
            return Storage.GetData();
        }

        PHX_FORCEINLINE uint32 GetCapacity() const
        {
            return Storage.GetCapacity();
        }

        PHX_FORCEINLINE uint32 GetNum() const
        {
            return Size;
        }

        PHX_FORCEINLINE bool IsValidIndex(uint32 index) const
        {
            return index < Size && Storage.IsValidIndex(index);
        }

        PHX_FORCEINLINE bool IsEmpty() const
        {
            return Size == 0;
        }

        PHX_FORCEINLINE bool IsFull() const
        {
            return Size == Storage.GetCapacity();
        }

        bool PushBack(const T& value = {})
        {
            PHX_ASSERT(!IsFull());
            if (IsFull())
            {
                return false;
            }
            Storage[Size++] = value;
            return true;
        }

        T& PushBack_GetRef(const T& value = {})
        {
            PHX_ASSERT(!IsFull());
            if (IsFull())
            {
                // TODO (jfarris): this should probably result in a crash!
                static T temp;
                return temp;
            }
            Storage[Size++] = value;
            return Storage[Size - 1];
        }

        template <class ...TArgs>
        bool EmplaceBack(TArgs&&... args)
        {
            PHX_ASSERT(!IsFull());
            if (IsFull())
            {
                return false;
            }
            new (&Storage[Size++]) T(std::forward<TArgs>(args)...);
            return true;
        }

        template <class ...TArgs>
        T& EmplaceBack_GetRef(TArgs&&... args)
        {
            PHX_ASSERT(!IsFull());
            if (!EmplaceBack(std::forward<TArgs>(args)...))
            {
                // TODO (jfarris): this should probably result in a crash!
                static T temp;
                return temp;
            }
            return Storage[Size-1];
        }

        bool Insert(const T& item, uint32 index)
        {
            if (IsFull())
            {
                return false;
            }

            // Shift items towards the back to insert a new item into the list
            for (uint32 i = Size; i > index; --i)
            {
                if (i == Storage.GetCapacity())
                {
                    // Storage is full, drop last item
                    continue;
                }

                Storage[i] = Storage[i - 1];
            }

            ++Size;

            Storage[index] = item;
            return true;
        }

        template <class ...TArgs>
        bool EmplaceInsert(uint32 index, TArgs&& ...args)
        {
            if (IsFull())
            {
                return false;
            }

            // Shift items towards the back to insert a new item into the list
            for (uint32 i = Size; i > index; --i)
            {
                if (i == Storage.GetCapacity())
                {
                    // Storage is full, drop last item
                    continue;
                }

                Storage[i] = Storage[i - 1];
            }

            ++Size;

            new (&Storage[index]) T(std::forward<TArgs>(args)...);
            return true;
        }

        void PopBack()
        {
            PHX_ASSERT(!IsEmpty());
            if (IsEmpty())
            {
                // TODO (jfarris): this should probably result in a crash?
                return;
            }
            Storage[Size--].~T();
        }

        T PopBackAndReturn()
        {
            PHX_ASSERT(!IsEmpty());
            if (IsEmpty())
            {
                // TODO (jfarris): this should probably result in a crash?
                return {};
            }
            T* data = Storage;
            T temp = data[Size - 1];
            data[Size--].~T();
            return temp;
        }

        T& Front()
        {
            PHX_ASSERT(!IsEmpty());
            if (IsEmpty())
            {
                // TODO (jfarris): this should probably result in a crash?
                static T temp;
                return temp;
            }
            return Storage[0];
        }

        T& Back()
        {
            PHX_ASSERT(!IsEmpty());
            if (IsEmpty())
            {
                // TODO (jfarris): this should probably result in a crash?
                static T temp;
                return temp;
            }
            return Storage[Size - 1];
        }

        void Reset()
        {
            SetNum(0);
        }

        void SetNum(uint32 newSize, const T& value = {})
        {
            newSize = std::min(newSize, Storage.GetCapacity());
            while (Size < newSize)
                PushBack(value);
            while (Size > newSize)
                PopBack();
        }

        void SetSize(uint32 newSize)
        {
            Size = std::min(newSize, Storage.GetCapacity());
        }

        void Fill(const T& value = {})
        {
            SetNum(Storage.GetCapacity(), value);
        }

        template <class TEquals = std::equal_to<T>>
        int32 IndexOf(const T& value, const TEquals& equals = {}) const
        {
            const T* data = Storage.GetData();
            for (uint32 i = 0; i < Size; ++i)
            {
                if (equals(data[i], value))
                    return (int32)i;
            }
            return INDEX_NONE;
        }

        template <class TEquals = std::equal_to<T>>
        bool Contains(const T& value, const TEquals& equals = {}) const
        {
            return IndexOf(value, equals) != INDEX_NONE;
        }

        bool RemoveAt(uint32 index)
        {
            if (!IsValidIndex(index))
            {
                return false;
            }

            for (uint32 i = index; i < Size - 1; ++i)
            {
                Storage[i] = Storage[i + 1];
            }

            --Size;
            return true;
        }

        bool RemoveAtUnsorted(uint32 index)
        {
            if (!IsValidIndex(index))
            {
                return false;
            }

            if (Size > 0)
            {
                Storage[index] = Storage[Size - 1];
            }

            --Size;
            return true;
        }

        struct Iter
        {
            using value_type = T;
            using element_type = T;
            using iterator_category = std::contiguous_iterator_tag;

            Iter(T* data = nullptr) : DataPtr(data) {}

            T* operator->() const
            {
                return DataPtr;
            }

            T& operator*() const
            {
                return *DataPtr;
            }

            T& operator[](int32 n) const
            {
                return *(DataPtr + n);
            }

            Iter& operator++()
            {
                ++DataPtr;
                return *this;
            }

            Iter operator++(int32 n) const
            {
                return { DataPtr + n };
            }

            Iter& operator--()
            {
                --DataPtr;
                return *this;
            }

            Iter operator--(int32 n) const
            {
                return { DataPtr - n };
            }

            Iter& operator+=(int64 n)
            {
                DataPtr += n;
                return *this;
            }

            Iter& operator-=(int64 n)
            {
                DataPtr -= n;
                return *this;
            }

            friend auto operator<=>(Iter, Iter) = default;

            friend auto operator-(const Iter& a, const Iter& b)
            {
                return a.DataPtr - b.DataPtr;
            }

            friend Iter operator+(Iter i, int64 n)
            {
                return { i.DataPtr + n };
            }

            friend Iter operator-(Iter i, int64 n)
            {
                return { i.DataPtr - n };
            }

            friend Iter operator+(int64 n, Iter i)
            {
                return { i.DataPtr + n };
            }

            bool operator==(const Iter& other) const
            {
                return DataPtr == other.DataPtr;
            }

        private:
            T* DataPtr;
        };

        Iter begin()
        {
            return Iter(Storage.GetData());
        }

        Iter end()
        {
            return Iter(Storage.GetData() + Size);
        }

        struct ConstIter
        {
            using value_type = T;
            using element_type = T;
            using iterator_category = std::contiguous_iterator_tag;

            ConstIter(const T* data = nullptr) : DataPtr(data) {}

            const T* operator->() const
            {
                return DataPtr;
            }

            const T& operator*() const
            {
                return *DataPtr;
            }

            const T& operator[](int32 n) const
            {
                return *(DataPtr + n);
            }

            ConstIter& operator++()
            {
                ++DataPtr;
                return *this;
            }

            ConstIter operator++(int32 n) const
            {
                return { DataPtr + n };
            }

            ConstIter& operator--()
            {
                --DataPtr;
                return *this;
            }

            ConstIter operator--(int32 n) const
            {
                return { DataPtr - n };
            }

            ConstIter& operator+=(int64 n)
            {
                DataPtr += n;
                return *this;
            }

            ConstIter& operator-=(int64 n)
            {
                DataPtr -= n;
                return *this;
            }

            friend auto operator<=>(ConstIter, ConstIter) = default;

            friend auto operator-(const ConstIter& a, const ConstIter& b)
            {
                return a.DataPtr - b.DataPtr;
            }

            friend ConstIter operator+(ConstIter i, int64 n)
            {
                return { i.DataPtr + n };
            }

            friend ConstIter operator-(ConstIter i, int64 n)
            {
                return { i.DataPtr - n };
            }

            friend ConstIter operator+(int64 n, ConstIter i)
            {
                return { i.DataPtr + n };
            }

            bool operator==(const ConstIter& other) const
            {
                return DataPtr == other.DataPtr;
            }

        private:
            const T* DataPtr;
        };

        ConstIter begin() const
        {
            return ConstIter(Storage.GetData());
        }

        ConstIter end() const
        {
            return ConstIter(Storage.GetData() + Size);
        }
    };

    template <class T>
    class TArray<T, HeapStoragePolicy> : THeapStorage<T>
    {
        using Super = THeapStorage<T>;
        using Super::Data;

    public:

        using Super::Super;
        using Super::CanGrow;
        using Super::GetCapacity;
        using Super::IsValidIndex;
        using Super::GetData;
        using Super::operator[];

        using Iter = decltype(std::declval<Super>().begin());
        using ConstIter = decltype(std::declval<const Super>().begin());

        TArray() = default;
        TArray(const TArray& other) = default;
        TArray(TArray&& other) = default;

        PHX_FORCEINLINE uint32 GetNum() const
        {
            return Data.size();
        }

        PHX_FORCEINLINE bool IsFull() const
        {
            return false;
        }

        PHX_FORCEINLINE bool IsEmpty() const
        {
            return Data.empty();
        }

        bool PushBack(const T& value) const
        {
            Data.push_back(value);
            return true;
        }

        template <class TEquals = std::equal_to<T>>
        bool PushBackUnique(const T& value, const TEquals& equals = {}) const
        {
            for (const T& existingItem : Data)
            {
                if (equals(existingItem, value))
                {
                    return false;
                }
            }
            Data.push_back(value);
            return true;
        }

        template <class ...TArgs>
        bool EmplaceBack(TArgs&&... args)
        {
            Data.emplace_back(std::forward<TArgs>(args)...);
            return true;
        }

        template <class ...TArgs>
        T& EmplaceBack_GetRef(TArgs&&... args)
        {
            return Data.emplace_back(std::forward<TArgs>(args)...);
        }

        bool Insert(uint32 index, const T& item)
        {
            Data.insert(Data.begin() + index, item);
            return true;
        }

        bool PopBack()
        {
            if (IsEmpty())
            {
                return false;
            }
            Data.pop_back();
            return true;
        }

        T& Front()
        {
            return Data.front();
        }

        const T& Front() const
        {
            return Data.front();
        }

        T& Back()
        {
            return Data.back();
        }

        const T& Back() const
        {
            return Data.back();
        }

        void Reset()
        {
            Data.clear();
        }

        void Reserve(uint32 newCapacity)
        {
            Data.reserve(newCapacity);
        }

        void SetNum(size_t newSize, const T& value = {})
        {
            while (Data.size() < newSize)
                Data.push_back(value);
            while (Data.size() > newSize)
                Data.pop_back();
        }

        void SetSize(uint32 newSize)
        {
            Data.resize(newSize);
        }

        int32 IndexOf(const T& value) const
        {
            auto iter = std::find(Data.begin(), Data.end(), value);
            if (iter == Data.end())
                return INDEX_NONE;
            return iter - Data.begin();
        }

        bool Contains(const T& value) const
        {
            return IndexOf(value) != INDEX_NONE;
        }

        auto RemoveAt(size_t index)
        {
            return Data.erase(Data.begin() + index);
        }

        auto begin()
        {
            return Data.begin();
        }

        auto begin() const
        {
            return Data.begin();
        }

        auto end()
        {
            return Data.end();
        }

        auto end() const
        {
            return Data.end();
        }

        TArray& operator=(const TArray& other)
        {
            Data = other.Data;
            return *this;
        }

        TArray& operator=(std::initializer_list<T> list)
        {
            Data = list;
            return *this;
        }

        TArray& operator=(TArray&& other) noexcept
        {
            Data = std::move(other.Data);
            return *this;
        }
    };

    template <class T, uint32 N>
    using TInlineArray = TArray<T, InlineStoragePolicy<N>>;

    template <class T>
    using TFixedArray = TArray<T, FixedStoragePolicy>;

    template <class T>
    using THeapArray = TArray<T, HeapStoragePolicy>;
}
