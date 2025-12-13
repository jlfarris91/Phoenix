
#pragma once

#include <algorithm>

#include "Platform.h"
#include "FixedArray.h"

namespace Phoenix
{
    template <class TItem, uint32 N, class TGetItemKey>
    class TFixedSortedList
    {
    public:

        static constexpr uint32 Capacity = N;
        static const TGetItemKey GetItemKey;
        using TKey = decltype(GetItemKey(TItem{}));

        uint32 GetSize() const
        {
            return static_cast<uint32>(Items.Num());
        }

        uint32 GetNumValidItems() const
        {
            return NumValidItems;
        }

        bool Contains(const TItem& item) const
        {
            return FindIndexOfItem(item) != Index<uint32>::None;
        }

        bool PushBack(const TItem& item)
        {
            if (!Items.Add(item))
            {
                return false;
            }

            if (Items.Back().IsValid())
            {
                ++NumValidItems;
            }

            return true;
        }

        bool PushBackUnique(const TItem& item)
        {
            if (Items.IsFull())
            {
                return false;
            }

            if (FindIndexOfItem(item) != Index<uint32>::None)
            {
                return false;
            }

            if (!Items.Add(item))
            {
                return false;
            }

            if (Items.Back().IsValid())
            {
                ++NumValidItems;
            }

            return true;
        }

        template <class ...TArgs>
        bool EmplaceBack(TArgs&&... args)
        {
            if (Items.IsFull())
            {
                return false;
            }

            if (!Items.EmplaceBack(std::forward<TArgs>(args)...))
            {
                return false;
            }

            if (Items.Back().IsValid())
            {
                ++NumValidItems;
            }

            return true;
        }

        bool Insert(uint32 index, const TItem& item)
        {
            if (Items.IsFull())
            {
                return false;
            }

            if (!InsertInternal(index, item))
            {
                return false;
            }

            // Item is being inserted into the sorted section so increase the number of sorted items.
            if (index < SortedNum)
            {
                ++SortedNum;
            }

            return true;
        }

        bool InsertSubItem(const TItem& item, uint32 subIndex)
        {
            if (Items.IsFull())
            {
                return false;
            }

            uint32 index = FindIndexOfSubItem(GetItemKey(item), subIndex);
            if (index == Index<uint32>::None)
            {
                return PushBack(item);
            }

            return InsertInternal(index, item);
        }

        template <class ...TArgs>
        bool EmplaceInsert(uint32 index, TArgs&&... args)
        {
            if (Items.IsFull())
            {
                return false;
            }

            if (!EmplaceInsertInternal(index, std::forward<TArgs>(args)...))
            {
                return false;
            }

            // Item is being inserted into the sorted section so increase the number of sorted items.
            if (index < SortedNum)
            {
                ++SortedNum;
            }

            return true;
        }

        bool Remove(const TItem& item)
        {
            if (Items.IsEmpty())
            {
                return false;
            }

            uint32 index = FindIndexOfItem(item);
            if (index == Index<uint32>::None)
            {
                return false;
            }

            TItem& existingItem = Items[index];
            existingItem.Invalidate();

            --NumValidItems;

            return true;
        }

        bool RemoveAt(uint32 index)
        {
            if (Items.IsEmpty())
            {
                return false;
            }

            if (!Items.IsValidIndex(index))
            {
                return false;
            }

            TItem& existingItem = Items[index];

            if (existingItem.IsValid())
            {
                --NumValidItems;
            }

            existingItem.Invalidate();

            return true;
        }

        bool RemoveAtAndReturn(uint32 index, TItem& outItem)
        {
            if (Items.IsEmpty())
            {
                return false;
            }

            if (!Items.IsValidIndex(index))
            {
                return false;
            }

            TItem& existingItem = Items[index];

            outItem = existingItem;

            if (existingItem.IsValid())
            {
                --NumValidItems;
            }

            existingItem.Invalidate();

            return true;
        }

        bool RemoveSubItem(const TKey& key, uint32 subIndex)
        {
            if (Items.IsEmpty())
            {
                return false;
            }

            TItem* item = FindSubItemInternal(key, subIndex);
            if (!item)
            {
                return false;
            }

            item->Invalidate();
            --NumValidItems;

            return true;
        }

        bool RemoveSubItemAndReturn(const TKey& key, uint32 subIndex, TItem& outItem)
        {
            if (Items.IsEmpty())
            {
                return false;
            }

            TItem* item = FindSubItemInternal(key, subIndex);
            if (!item)
            {
                return false;
            }

            outItem = *item;

            item->Invalidate();
            --NumValidItems;

            return true;
        }

        uint32 RemoveAll(const TKey& key)
        {
            if (Items.IsEmpty())
            {
                return false;
            }

            uint32 numRemoved = 0;

            auto begin = Items.begin();
            auto end = Items.end();
            auto sortedEnd = begin + SortedNum;

            PHX_ASSERT(sortedEnd <= end);

            // Search the sorted section
            if (sortedEnd != begin)
            {
                auto iter = std::lower_bound(begin, sortedEnd, TItem(key), SortItemsByKey());
                while (iter != sortedEnd && GetItemKey(*iter) == key)
                {
                    TItem& item = *iter;
                    if (item.IsValid())
                    {
                        item.Invalidate();
                        ++numRemoved;
                        --NumValidItems;
                    }
                    ++iter;
                }
            }

            // Search the unsorted section
            auto iter = sortedEnd;
            while (iter < end)
            {
                TItem& item = *iter;
                if (GetItemKey(item) == key && item.IsValid())
                {
                    item.Invalidate();
                    ++numRemoved;
                    --NumValidItems;
                }
                ++iter;
            }

            return numRemoved;
        }

        // Removes all items matching a given predicate. Note that this operation is O(N).
        template <class TPredicate>
        uint32 RemoveAll(const TPredicate& pred)
        {
            if (Items.IsEmpty())
            {
                return false;
            }

            uint32 numRemoved = 0;

            for (TItem& item : Items)
            {
                if (item.IsValid() && pred(item))
                {
                    item.Invalidate();
                    --NumValidItems;
                    ++numRemoved;
                }
            }

            return numRemoved;
        }

        const TItem* GetFirstSubItem(const TKey& key, uint32& outIndex) const
        {
            if (Items.IsEmpty())
            {
                return nullptr;
            }

            auto begin = Items.begin();
            auto end = Items.end();
            auto sortedEnd = begin + SortedNum;

            PHX_ASSERT(sortedEnd <= end);

            // Search the sorted section
            if (sortedEnd != begin)
            {
                auto iter = std::lower_bound(begin, sortedEnd, TItem(key), SortItemsByKey());
                while (iter != sortedEnd && GetItemKey(*iter) == key)
                {
                    const TItem& item = *iter;
                    if (item.IsValid())
                    {
                        outIndex = static_cast<uint32>(iter - begin);
                        return &item;
                    }
                    ++iter;
                }
            }

            // Search the unsorted section
            auto iter = sortedEnd;
            while (iter < end)
            {
                const TItem& item = *iter;
                if (GetItemKey(item) == key && item.IsValid())
                {
                    outIndex = static_cast<uint32>(iter - begin);
                    return &item;
                }
                ++iter;
            }

            return nullptr;
        }

        const TItem* GetNextSubItem(const TKey& key, uint32 currIndex, uint32& outIndex) const
        {
            if (Items.IsEmpty())
            {
                return nullptr;
            }

            uint32 index = currIndex + 1;

            // Search the sorted section
            while (index < SortedNum && GetItemKey(Items[index]) == key)
            {
                const TItem& item = Items[index];
                if (item.IsValid())
                {
                    outIndex = index;
                    return &item;
                }
                ++index;
            }

            // Search the unsorted section
            index = SortedNum;
            while (index < Items.Num())
            {
                const TItem& item = Items[index];
                if (GetItemKey(item) == key && item.IsValid())
                {
                    outIndex = index;
                    return &item;
                }
                ++index;
            }

            outIndex = Index<uint32>::None;
            return nullptr;
        }

        const TItem* GetSubItem(const TKey& key, uint32 subIndex) const
        {
            if (Items.IsEmpty())
            {
                return nullptr;
            }

            return FindSubItemInternal(key, subIndex);
        }

        uint32 GetNumSubItems(const TKey& key) const
        {
            if (Items.IsEmpty())
            {
                return 0;
            }

            auto begin = Items.begin();
            auto end = Items.end();
            auto sortedEnd = begin + SortedNum;
            uint32 numItems = 0;

            PHX_ASSERT(sortedEnd <= end);

            // Search the sorted section
            if (sortedEnd != begin)
            {
                auto iter = std::lower_bound(begin, sortedEnd, TItem(key), SortItemsByKey());
                while (iter != sortedEnd && GetItemKey(*iter) == key)
                {
                    if (iter->IsValid())
                    {
                        ++numItems;
                    }
                    ++iter;
                }
            }

            // Search the unsorted section
            auto iter = sortedEnd;
            while (iter < end)
            {
                const TItem& item = *iter;
                if (GetItemKey(item) == key && item.IsValid())
                {
                    ++numItems;
                }
                ++iter;
            }

            return numItems;
        }

        // Executes a callback for each item in the list.
        // Note that this operation is O(N).
        template <class TCallback>
        void ForEachItem(const TCallback& callback) const
        {
            if (Items.IsEmpty())
            {
                return;
            }

            for (const TItem& item : Items)
            {
                if (item.IsValid())
                {
                    callback(item);
                }
            }
        }

        // Executes a callback for each item in the list.
        // Note that this operation is O(N).
        template <class TPredicate, class TCallback>
        void ForEachItem(const TPredicate& pred, const TCallback& callback) const
        {
            if (Items.IsEmpty())
            {
                return;
            }

            for (const TItem& item : Items)
            {
                if (item.IsValid() && pred(item))
                {
                    callback(item);
                }
            }
        }

        template <class TCallback>
        void ForEachSubItem(const TKey& key, const TCallback& callback) const
        {
            if (Items.IsEmpty())
            {
                return;
            }

            auto begin = Items.begin();
            auto end = Items.end();
            auto sortedEnd = begin + SortedNum;

            PHX_ASSERT(sortedEnd <= end);

            // Search the sorted section
            if (sortedEnd != begin)
            {
                auto iter = std::lower_bound(begin, sortedEnd, TItem(key), SortItemsByKey());
                while (iter != sortedEnd && GetItemKey(*iter) == key)
                {
                    const TItem& item = *iter;
                    if (item.IsValid())
                    {
                        callback(item);
                    }
                    ++iter;
                }
            }

            // Search the unsorted section
            auto iter = sortedEnd;
            while (iter < end)
            {
                const TItem& item = *iter;
                if (GetItemKey(item) == key && item.IsValid())
                {
                    callback(item);
                }
                ++iter;
            }
        }

        void Sort()
        {
            if (Items.IsEmpty())
            {
                return;
            }

            std::stable_sort(Items.begin(), Items.end(), SortInvalidItemsToBack());

            auto iter = Items.end() - 1;
            while (iter != Items.begin() && !iter->IsValid())
            {
                --iter;
            }

            SortedNum = static_cast<uint32>(iter - Items.begin());

            if (iter->IsValid())
            {
                ++SortedNum;
            }

            SortedNum = std::min(SortedNum, Capacity);

            Items.SetSize(SortedNum);
        }

    private:

        struct SortItemsByKey
        {
            constexpr bool operator()(const TItem& a, const TItem& b) const
            {
                return GetItemKey(a) < GetItemKey(b);
            }
        };

        struct SortInvalidItemsToBack
        {
            constexpr bool operator()(const TItem& a, const TItem& b) const
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
                return GetItemKey(a) < GetItemKey(b);
            }
        };

        struct InvalidItemPred
        {
            constexpr bool operator()(const TItem& item) const
            {
                return !item.IsValid();
            }
        };

        uint32 FindIndexOfItem(const TItem& item) const
        {
            auto begin = Items.begin();
            auto end = Items.end();
            auto sortedEnd = begin + SortedNum;

            PHX_ASSERT(sortedEnd <= end);

            // Search the sorted section
            if (sortedEnd != begin)
            {
                auto iter = std::lower_bound(begin, sortedEnd, item, SortItemsByKey());
                while (iter != sortedEnd && GetItemKey(*iter) == GetItemKey(item))
                {
                    if (*iter == item)
                    {
                        return static_cast<uint32>(iter - begin);
                    }
                    ++iter;
                }
            }

            // Search the unsorted section
            auto iter = sortedEnd;
            while (iter < end)
            {
                if (*iter == item)
                {
                    return static_cast<uint32>(iter - begin);
                }
                ++iter;
            }

            return Index<uint32>::None;
        }

        uint32 FindIndexOfSubItem(const TKey& key, uint32 index) const
        {
            auto begin = Items.begin();
            auto end = Items.end();
            auto sortedEnd = begin + SortedNum;

            PHX_ASSERT(sortedEnd <= end);

            // Search the sorted section
            if (sortedEnd != begin)
            {
                auto iter = std::lower_bound(begin, sortedEnd, TItem(key), SortItemsByKey());
                while (iter != sortedEnd && GetItemKey(*iter) == key)
                {
                    if (iter->IsValid() && index-- == 0)
                    {
                        return static_cast<uint32>(iter - begin);
                    }
                    ++iter;
                }
            }

            // Search the unsorted section
            auto iter = sortedEnd;
            while (iter < end)
            {
                const TItem& item = *iter;
                if (GetItemKey(item) == key && item.IsValid() && index-- == 0)
                {
                    return static_cast<uint32>(iter - begin);
                }
                ++iter;
            }

            return Index<uint32>::None;
        }

        TItem* FindSubItemInternal(const TKey& key, uint32 subIndex)
        {
            uint32 index = FindIndexOfSubItem(key, subIndex);
            return index == Index<uint32>::None ? nullptr : &Items[index];
        }

        const TItem* FindSubItemInternal(const TKey& key, uint32 subIndex) const
        {
            uint32 index = FindIndexOfSubItem(key, subIndex);
            return index == Index<uint32>::None ? nullptr : &Items[index];
        }

        bool ShiftItemsTowardsBack(uint32 index)
        {
            // Find the next invalid item slot.
            uint32 i = index;
            for (; i < Items.Num(); ++i)
            {
                if (!Items[i].IsValid())
                {
                    break;
                }
            }

            // Make room for a new item if there was no invalid item slot.
            if (i == Items.Num() && !Items.AddDefaulted())
            {
                // Queue is full, can't add any new items.
                return false;
            }

            // Shift all items back towards the invalid slot.
            uint32 j = i;
            for (; j > index; --j)
            {
                Items[j] = Items[j - 1];
            }

            PHX_ASSERT(j == index);
            return true;
        }

        bool InsertInternal(uint32 index, const TItem& item)
        {
            PHX_ASSERT(index < Capacity);

            if (!ShiftItemsTowardsBack(index))
            {
                return false;
            }

            bool wasValid = Items[index].IsValid();

            Items[index] = item;

            if (!wasValid && Items[index].IsValid())
            {
                ++NumValidItems;
            }
            else if (wasValid && !Items[index].IsValid())
            {
                --NumValidItems;
            }

            return true;
        }

        template <class ...TArgs>
        bool EmplaceInsertInternal(uint32 index, TArgs&&... args)
        {
            PHX_ASSERT(index < Capacity);

            if (!ShiftItemsTowardsBack(index))
            {
                return false;
            }

            bool wasValid = Items[index].IsValid();

            new (&Items[index]) TItem(std::forward<TArgs>(args)...);

            if (!wasValid && Items[index].IsValid())
            {
                ++NumValidItems;
            }
            else if (wasValid && !Items[index].IsValid())
            {
                --NumValidItems;
            }

            return true;
        }

        TFixedArray<TItem, Capacity> Items;
        uint32 SortedNum = 0;
        uint32 NumValidItems = 0;
    };

    template <class TItem, uint32 N, class TGetItemKey>
    const TGetItemKey TFixedSortedList<TItem, N, TGetItemKey>::GetItemKey = {};
}
