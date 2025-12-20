
#pragma once

#include "PhoenixSim/Platform.h"

namespace Phoenix
{
    // This just wraps std::vector for now.
    // Matches the API of TFixedArray.
    template <class T>
    struct TArray2
    {
        T& operator[](size_t index)
        {
            return Data[index];
        }

        const T& operator[](size_t index) const
        {
            return Data[index];
        }

        size_t Num() const
        {
            return Data.size();
        }

        bool IsValidIndex(size_t index) const
        {
            return index < Data.size();
        }

        bool IsEmpty() const
        {
            return Data.empty();
        }

        // Matching TFixedArray API but can't be full.
        bool IsFull() const
        {
            return false;
        }

        T& PushBack(const T& value)
        {
            Data.push_back(value);
            return Data.back();
        }

        T& PushBackUnique(const T& item)
        {
            for (T& existingItem : Data)
            {
                if (existingItem == item)
                {
                    return existingItem;
                }
            }

            return PushBack(item);
        }

        void Add(const T& value)
        {
            (void)PushBack(value);
        }

        T& Add_GetRef(const T& value)
        {
            return PushBack(value);
        }

        T& AddDefaulted_GetRef()
        {
            return PushBack({});
        }

        template <class ...TArgs>
        T& EmplaceBack_GetRef(TArgs&&... args)
        {
            return Data.emplace_back(std::forward<TArgs>(args)...);
        }

        template <class ...TArgs>
        void EmplaceBack(TArgs&&... args)
        {
            (void)EmplaceBack_GetRef(std::forward<TArgs>(args)...);
        }

        void PopBack()
        {
            Data.pop_back();
        }

        T PopBackAndReturn()
        {
            T temp = Data.back();
            Data.pop_back();
            return temp;
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

        std::vector<T> Data;
    };
}
