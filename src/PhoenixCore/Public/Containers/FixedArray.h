#pragma once

#include <cassert>
#include <iterator>

namespace Phoenix
{
    template <class T, uint32 N>
    class TFixedArray
    {
    public:
        using ItemT = T;
        static constexpr uint32 Capacity = N;

        T& operator[](uint32 index)
        {
            PHX_ASSERT(index < Capacity);
            return *(Data + index);
        }

        const T& operator[](uint32 index) const
        {
            PHX_ASSERT(index < Capacity);
            return *(Data + index);
        }

        uint32 Num() const
        {
            return Size;
        }

        uint32 GetTotalSize() const
        {
            return Size * sizeof(T);
        }

        bool IsValidIndex(uint32 index) const
        {
            return index < Size && index < Capacity;
        }

        bool IsEmpty() const
        {
            return Size == 0;
        }

        bool IsFull() const
        {
            return Size == Capacity;
        }

        bool PushBack(const T& value)
        {
            PHX_ASSERT(!IsFull());
            if (IsFull())
            {
                return false;
            }
            Data[Size++] = value;
            return true;
        }

        bool Add(const T& value)
        {
            return PushBack(value);
        }

        T& Add_GetRef(const T& value)
        {
            if (!PushBack(value))
            {
                static T temp;
                return temp;
            }
            return Data[Size-1];
        }

        bool AddDefaulted()
        {
            return Add({});
        }

        T& AddDefaulted_GetRef()
        {
            return Add_GetRef({});
        }

        template <class ...TArgs>
        bool EmplaceBack(TArgs&&... args)
        {
            PHX_ASSERT(!IsFull());
            if (IsFull())
            {
                return false;
            }
            new (&Data[Size++]) T(std::forward<TArgs>(args)...);
            return true;
        }

        template <class ...TArgs>
        T& EmplaceBack_GetRef(TArgs&&... args)
        {
            PHX_ASSERT(Size < Capacity);
            if (!EmplaceBack(std::forward<TArgs>(args)...))
            {
                static T temp;
                return temp;
            }
            return Data[Size-1];
        }

        void PopBack()
        {
            PHX_ASSERT(Size > 0);
            if (Size == 0)
            {
                return;
            }
            Data[Size--].~T();
        }

        T PopBackAndReturn()
        {
            PHX_ASSERT(Size > 0);
            if (Size == 0)
            {
                return {};
            }
            T temp = Data[Size - 1];
            Data[Size--].~T();
            return temp;
        }

        T& Front()
        {
            PHX_ASSERT(!IsEmpty());
            if (IsEmpty())
            {
                static T temp;
                return temp;
            }
            return Data[0];
        }

        const T& Front() const
        {
            PHX_ASSERT(!IsEmpty());
            if (IsEmpty())
            {
                static T temp;
                return temp;
            }
            return Data[0];
        }

        T& Back()
        {
            PHX_ASSERT(!IsEmpty());
            if (IsEmpty())
            {
                static T temp;
                return temp;
            }
            return Data[Size - 1];
        }

        const T& Back() const
        {
            PHX_ASSERT(!IsEmpty());
            if (IsEmpty())
            {
                static T temp;
                return temp;
            }
            return Data[Size - 1];
        }

        void Reset()
        {
            SetNum(0);
        }

        void SetNum(uint32 newSize, const T& value = {})
        {
            newSize = std::min(newSize, Capacity);
            while (Size < newSize)
                Add_GetRef(value);
            while (Size > newSize)
                PopBack();
        }

        void SetSize(uint32 newSize)
        {
            Size = std::min(newSize, Capacity);
        }

        void Fill(const T& value = {})
        {
            SetNum(Capacity, value);
        }

        int32 IndexOf(const T& value)
        {
            for (uint32 i = 0; i < Size; ++i)
            {
                if (Data[i] == value)
                    return (int32)i;
            }
            return INDEX_NONE;
        }

        bool Contains(const T& value)
        {
            return IndexOf(value) != INDEX_NONE;
        }

        void RemoveAt(uint32 index)
        {
            if (!IsValidIndex(index))
            {
                return;
            }

            for (uint32 i = index; i < Size - 1; ++i)
            {
                Data[i] = Data[i + 1];
            }

            --Size;
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
            return Iter(Data);
        }

        Iter end()
        {
            return Iter(Data + Size);
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
            return ConstIter(Data);
        }

        ConstIter end() const
        {
            return ConstIter(Data + Size);
        }

    private:
        T Data[N];
        uint32 Size = 0;
    };

    static_assert(std::contiguous_iterator<TFixedArray<int, 1>::Iter>);
    static_assert(std::contiguous_iterator<TFixedArray<int, 1>::ConstIter>);
}
