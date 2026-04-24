

#pragma once

#include <algorithm>

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Utils.h"
#include "PhoenixSim/Containers/FixedArray.h"
#include "PhoenixSim/Containers/FixedMemory.h"

namespace Phoenix
{
    template <class T, class TStoragePolicy>
    class TSortedListBase
    {
    protected:
        using TStorage = TArray<T, TStoragePolicy>;
        TStorage Storage;
        uint32 SortedNum = 0;
        uint32 NumValidItems = 0;
    };

    template <class T>
    class TSortedListBase<T, FixedStoragePolicy>
    {
    public:

        TSortedListBase() = default;

        template <class TAllocator>
        TSortedListBase(TAllocator& allocator, uint32 capacity)
            : Storage(allocator, capacity)
        {
        }

        template <class TAllocator, class TOtherStoragePolicy>
        TSortedListBase(TAllocator& allocator, uint32 capacity, const TSortedListBase<T, TOtherStoragePolicy>& other)
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
        using TStorage = TFixedArray<T>;
        TStorage Storage;
        uint32 SortedNum = 0;
        uint32 NumValidItems = 0;
    };
    
    template <class T, class TGetItemKey, class TStoragePolicy>
    class TSortedList : public TSortedListBase<T, TStoragePolicy>
    {
        using Super = TSortedListBase<T, TStoragePolicy>;
        using Super::Super;
        using Super::Storage;
        using Super::SortedNum;
        using Super::NumValidItems;

        using TStorage = typename Super::TStorage;
        using TItemsIter = typename TStorage::Iter;
        using TItemsConstIter = typename TStorage::ConstIter;

    public:

        static const TGetItemKey GetKey;
        using TKey = decltype(GetKey(T{}));

        struct SortedItemRun
        {
            uint32 Start;
            uint32 End;
        };

        PHX_FORCEINLINE uint32 GetCapacity() const
        {
            return Storage.GetCapacity();
        }

        PHX_FORCEINLINE uint32 GetNum() const
        {
            return Storage.GetNum();
        }

        PHX_FORCEINLINE uint32 GetNumValidItems() const
        {
            return NumValidItems;
        }

        PHX_FORCEINLINE uint32 GetSortedNum() const
        {
            return SortedNum;
        }

        PHX_FORCEINLINE bool IsEmpty() const
        {
            return Storage.IsEmpty();
        }

        PHX_FORCEINLINE bool IsFull() const
        {
            return Storage.IsFull();
        }

        PHX_FORCEINLINE T* GetData()
        {
            return Storage.GetData();
        }

        PHX_FORCEINLINE const T* GetData() const
        {
            return Storage.GetData();
        }

        PHX_FORCEINLINE bool IsValidIndex(uint32 index) const
        {
            return Storage.IsValidIndex(index);
        }

        template <class TItemEquals = std::equal_to<T>, class TKeyEquals = std::equal_to<TKey>>
        bool Contains(const T& item, const TItemEquals& itemEquals = {}, const TKeyEquals& keyEquals = {}) const
        {
            return FindIndexOfItem(item, itemEquals, keyEquals) != Index<uint32>::None;
        }

        bool PushBack(const T& item)
        {
            if (!Storage.PushBack(item))
            {
                return false;
            }

            if (Storage.Back().IsValid())
            {
                ++NumValidItems;
            }

            return true;
        }

        template <class TItemEquals = std::equal_to<T>, class TKeyEquals = std::equal_to<TKey>>
        bool PushBackUnique(const T& item, const TItemEquals& itemEquals = {}, const TKeyEquals& keyEquals = {})
        {
            if (Storage.IsFull())
            {
                return false;
            }

            if (FindIndexOfItem(item, itemEquals, keyEquals) != Index<uint32>::None)
            {
                return false;
            }

            if (!Storage.PushBack(item))
            {
                return false;
            }

            if (Storage.Back().IsValid())
            {
                ++NumValidItems;
            }

            return true;
        }

        template <class ...TArgs>
        bool EmplaceBack(TArgs&&... args)
        {
            if (Storage.IsFull())
            {
                return false;
            }

            if (!Storage.EmplaceBack(std::forward<TArgs>(args)...))
            {
                return false;
            }

            if (Storage.Back().IsValid())
            {
                ++NumValidItems;
            }

            return true;
        }

        bool Insert(uint32 index, const T& item)
        {
            return InsertInternal(index, item);
        }

        template <class ...TArgs>
        bool EmplaceInsert(uint32 index, TArgs&&... args)
        {
            return EmplaceInsertInternal(index, std::forward<TArgs>(args)...);
        }

        // TODO (jfarris): this is confusing...
        template <class TKeyEquals = std::equal_to<TKey>>
        bool InsertItem(const T& item, uint32 subIndex, const TKeyEquals& keyEquals = {})
        {
            if (Storage.IsFull())
            {
                return false;
            }

            uint32 index = FindIndexOfSubItemWithKey(GetKey(item), subIndex, keyEquals);
            if (index == Index<uint32>::None)
            {
                return PushBack(item);
            }

            return InsertInternal(index, item);
        }

        // TODO (jfarris): this is confusing...
        template <class ...TArgs, class TKeyEquals = std::equal_to<TKey>>
        bool EmplaceInsertItem(const T& item, uint32 subIndex, TArgs&&... args, const TKeyEquals& keyEquals = {})
        {
            if (Storage.IsFull())
            {
                return false;
            }

            uint32 index = FindIndexOfSubItemWithKey(GetKey(item), subIndex, keyEquals);
            if (index == Index<uint32>::None)
            {
                return EmplaceBack(std::forward<TArgs>(args)...);
            }

            return EmplaceInsertInternal(index, std::forward<TArgs>(args)...);
        }

        template <class TItemEquals = std::equal_to<T>, class TKeyEquals = std::equal_to<TKey>>
        bool Remove(const T& item, const TItemEquals& itemEquals = {}, const TKeyEquals& keyEquals = {})
        {
            if (Storage.IsEmpty())
            {
                return false;
            }

            uint32 index = FindIndexOfItem(item, itemEquals, keyEquals);
            if (index == Index<uint32>::None)
            {
                return false;
            }

            T& existingItem = Storage[index];
            if (!existingItem.IsValid())
            {
                return false;
            }

            existingItem.Invalidate();
            --NumValidItems;

            return true;
        }

        bool RemoveAt(uint32 index)
        {
            if (Storage.IsEmpty())
            {
                return false;
            }

            if (!Storage.IsValidIndex(index))
            {
                return false;
            }

            T& existingItem = Storage[index];
            if (!existingItem.IsValid())
            {
                return false;
            }

            existingItem.Invalidate();
            --NumValidItems;

            return true;
        }

        bool RemoveAt(uint32 index, T& outItem)
        {
            if (Storage.IsEmpty())
            {
                return false;
            }

            if (!Storage.IsValidIndex(index))
            {
                return false;
            }

            T& existingItem = Storage[index];
            if (!existingItem.IsValid())
            {
                return false;
            }

            outItem = existingItem;
            existingItem.Invalidate();
            --NumValidItems;

            return true;
        }

        template <class TKeyEquals = std::equal_to<TKey>>
        bool RemoveFirstItemByKey(const TKey& key, const TKeyEquals& keyEquals = {})
        {
            return RemoveItemByKey(key, 0, keyEquals);
        }

        template <class TKeyEquals = std::equal_to<TKey>>
        bool RemoveItemByKey(const TKey& key, uint32 subIndex, const TKeyEquals& keyEquals = {})
        {
            if (Storage.IsEmpty())
            {
                return false;
            }

            uint32 index;
            T* item = FindSubItemInternal(key, subIndex, index, keyEquals);
            if (!item || !item->IsValid())
            {
                return false;
            }

            item->Invalidate();
            --NumValidItems;

            return true;
        }

        template <class TKeyEquals = std::equal_to<TKey>>
        bool RemoveFirstItemByKey(const TKey& key, T& outItem, const TKeyEquals& keyEquals = {})
        {
            return RemoveItemByKey(key, 0, outItem, keyEquals);
        }

        template <class TKeyEquals = std::equal_to<TKey>>
        bool RemoveItemByKey(const TKey& key, uint32 subIndex, T& outItem, const TKeyEquals& keyEquals = {})
        {
            if (Storage.IsEmpty())
            {
                return false;
            }

            uint32 index;
            T* item = FindSubItemInternal(key, subIndex, index, keyEquals);
            if (!item || !item->IsValid())
            {
                return false;
            }

            outItem = *item;
            item->Invalidate();
            --NumValidItems;

            return true;
        }

        template <class TKeyEquals = std::equal_to<TKey>>
        uint32 RemoveAll(const TKey& key, const TKeyEquals& keyEquals = {})
        {
            if (Storage.IsEmpty())
            {
                return 0;
            }

            uint32 numRemoved = 0;
            uint32 index = FindIndexOfFirstSubItemWithKey(key, keyEquals);
            while (index != Index<uint32>::None)
            {
                Storage[index].Invalidate();
                ++numRemoved;
                --NumValidItems;

                index = FindIndexOfNextItemWithKey(key, index, keyEquals);
            }

            return numRemoved;
        }

        template <class TPredicate, class TKeyEquals = std::equal_to<TKey>>
        uint32 RemoveAll(const TKey& key, const TPredicate& pred, const TKeyEquals& keyEquals)
        {
            if (Storage.IsEmpty())
            {
                return 0;
            }

            uint32 numRemoved = 0;
            uint32 index = FindIndexOfFirstSubItemWithKey(key, keyEquals);
            while (index != Index<uint32>::None)
            {
                if (pred(Storage[index]))
                {
                    Storage[index].Invalidate();
                    ++numRemoved;
                    --NumValidItems;
                }
                index = FindIndexOfNextItemWithKey(key, index, keyEquals);
            }

            return numRemoved;
        }

        // Removes all items matching a given predicate. Note that this operation is O(N).
        template <class TPredicate>
        uint32 RemoveAll(const TPredicate& pred)
        {
            if (Storage.IsEmpty())
            {
                return false;
            }

            uint32 numRemoved = 0;

            for (T& item : Storage)
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

        // IMPORTANT:
        // This returns a non-const pointer to an item in the list for convenience of changing non-key values.
        // This may return a pointer to an item that is part of the sorted range, but you won't know that.
        // So, if you change the KEY used to sort this item the list may become unsorted and subsequent lookups will fail.
        // If you do change the key then you need to manually call Sort() afterwards.
        template <class TKeyEquals = std::equal_to<TKey>>
        T* GetFirstItem(const TKey& key, uint32& outIndex, const TKeyEquals& keyEquals = {})
        {
            return FindSubItemInternal(key, 0, outIndex, keyEquals);
        }

        template <class TKeyEquals = std::equal_to<TKey>>
        const T* GetFirstItem(const TKey& key, uint32& outIndex, const TKeyEquals& keyEquals = {}) const
        {
            return FindSubItemInternal(key, 0, outIndex, keyEquals);
        }

        // IMPORTANT:
        // This returns a non-const pointer to an item in the list for convenience of changing non-key values.
        // This may return a pointer to an item that is part of the sorted range, but you won't know that.
        // So, if you change the KEY used to sort this item the list may become unsorted and subsequent lookups will fail.
        // If you do change the key then you need to manually call Sort() afterwards.
        template <class TKeyEquals = std::equal_to<TKey>>
        T* GetNextItem(const TKey& key, uint32 currIndex, uint32& outIndex, const TKeyEquals& keyEquals = {})
        {
            return FindNextItemInternal(key, currIndex, outIndex, keyEquals);
        }

        template <class TKeyEquals = std::equal_to<TKey>>
        const T* GetNextItem(const TKey& key, uint32 currIndex, uint32& outIndex, const TKeyEquals& keyEquals = {}) const
        {
            return FindNextItemInternal(key, currIndex, outIndex, keyEquals);
        }

        // IMPORTANT:
        // This returns a non-const pointer to an item in the list for convenience of changing non-key values.
        // This may return a pointer to an item that is part of the sorted range, but you won't know that.
        // So, if you change the KEY used to sort this item the list may become unsorted and subsequent lookups will fail.
        // If you do change the key then you need to manually call Sort() afterwards.
        template <class TKeyEquals = std::equal_to<TKey>>
        T* GetItem(const TKey& key, uint32 subIndex = 0, const TKeyEquals& keyEquals = {})
        {
            uint32 index;
            return FindSubItemInternal(key, subIndex, index, keyEquals);
        }

        template <class TKeyEquals = std::equal_to<TKey>>
        const T* GetItem(const TKey& key, uint32 subIndex = 0, const TKeyEquals& keyEquals = {}) const
        {
            uint32 index;
            return FindSubItemInternal(key, subIndex, index, keyEquals);
        }

        template <class TKeyEquals = std::equal_to<TKey>>
        uint32 GetNumItems(const TKey& key, const TKeyEquals& keyEquals = {}) const
        {
            if (Storage.IsEmpty())
            {
                return 0;
            }

            TItemsConstIter begin = Storage.begin();
            TItemsConstIter end = Storage.end();
            TItemsConstIter sortedEnd = begin + SortedNum;
            uint32 numItems = 0;

            PHX_ASSERT(sortedEnd <= end);

            // Search the sorted section
            if (sortedEnd != begin)
            {
                uint32 index = FindIndexOfFirstSubItemWithKeyInSortedRange(key, keyEquals);
                while (index < SortedNum && keyEquals(GetKey(Storage[index]), key))
                {
                    if (Storage[index].IsValid())
                    {
                        ++numItems;
                    }
                    ++index;
                }
            }

            // Search the unsorted section
            TItemsConstIter iter = sortedEnd;
            while (iter < end)
            {
                const T& item = *iter;
                if (keyEquals(GetKey(item), key) && item.IsValid())
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
        void ForEachItem(const TCallback& callback)
        {
            if (Storage.IsEmpty())
            {
                return;
            }

            for (T& item : Storage)
            {
                if (item.IsValid() && InvokeForEachCallbackNoIndex(callback, item))
                {
                    return;
                }
            }
        }

        // Executes a callback for each const item in the list.
        // Note that this operation is O(N).
        template <class TCallback>
        void ForEachItem(const TCallback& callback) const
        {
            if (Storage.IsEmpty())
            {
                return;
            }

            for (const T& item : Storage)
            {
                if (item.IsValid() && InvokeForEachCallbackNoIndex(callback, item))
                {
                    return;
                }
            }
        }

        // Executes a callback for each item in the list that passes a predicate.
        // Note that this operation is O(N).
        template <class TPredicate, class TCallback>
        void ForEachItem(const TPredicate& pred, const TCallback& callback)
        {
            if (Storage.IsEmpty())
            {
                return;
            }

            for (T& item : Storage)
            {
                if (item.IsValid() && pred(item) && InvokeForEachCallbackNoIndex(callback, item))
                {
                    return;
                }
            }
        }

        // Executes a callback for each const item in the list that passes a predicate.
        // Note that this operation is O(N).
        template <class TPredicate, class TCallback>
        void ForEachItem(const TPredicate& pred, const TCallback& callback) const
        {
            if (Storage.IsEmpty())
            {
                return;
            }

            for (const T& item : Storage)
            {
                if (item.IsValid() && pred(item) && InvokeForEachCallbackNoIndex(callback, item))
                {
                    return;
                }
            }
        }

        template <class TProjection, class TCallback, class TKeyEquals = std::equal_to<TKey>>
        void ForEachItemProjected(
            const TKey& key,
            const TProjection& project,
            const TCallback& callback,
            const TKeyEquals& keyEquals = {}) const
        {
            if (Storage.IsEmpty())
            {
                return;
            }

            TItemsConstIter begin = Storage.begin();
            TItemsConstIter end = Storage.end();
            TItemsConstIter sortedEnd = begin + SortedNum;

            PHX_ASSERT(sortedEnd <= end);

            uint32 subIndex = 0;

            // Search the sorted section
            if (sortedEnd != begin)
            {
                uint32 index = FindIndexOfFirstSubItemWithKeyInSortedRange(key, keyEquals);
                while (index < SortedNum && keyEquals(GetKey(Storage[index]), key))
                {
                    const T& item = Storage[index];
                    if (item.IsValid())
                    {
                        if (InvokeForEachCallbackWithIndex(callback, subIndex, project(item)))
                        {
                            return;
                        }
                        ++subIndex;
                    }
                    ++index;
                }
            }

            // Search the unsorted section
            TItemsConstIter iter = sortedEnd;
            while (iter < end)
            {
                const T& item = *iter;
                if (keyEquals(GetKey(item), key) && item.IsValid())
                {
                    if (InvokeForEachCallbackWithIndex(callback, subIndex, project(item)))
                    {
                        return;
                    }
                    ++subIndex;
                }
                ++iter;
            }
        }

        template <class TCallback, class TKeyEquals = std::equal_to<TKey>>
        void ForEachItem(const TKey& key, const TCallback& callback, const TKeyEquals& keyEquals = {}) const
        {
            ForEachItemProjected(key, DefaultProjection(), callback, keyEquals);
        }

        void Reset()
        {
            Storage.Reset();
            SortedNum = 0;
            NumValidItems = 0;
        }

        void Sort()
        {
            if (Storage.IsEmpty())
            {
                return;
            }

            std::stable_sort(Storage.begin(), Storage.end(), SortInvalidItemsToBack());

            TItemsIter iter = Storage.end() - 1;
            while (iter != Storage.begin() && !iter->IsValid())
            {
                --iter;
            }

            SortedNum = static_cast<uint32>(iter - Storage.begin());

            if (iter->IsValid())
            {
                ++SortedNum;
            }

            SortedNum = std::min(SortedNum, Storage.GetCapacity());

            Storage.SetSize(SortedNum);
        }

        auto begin()
        {
            return Storage.begin();
        }

        auto begin() const
        {
            return Storage.begin();
        }

        auto end()
        {
            return Storage.end();
        }

        auto end() const
        {
            return Storage.end();
        }

    protected:

        struct SortInvalidItemsToBack
        {
            constexpr bool operator()(const T& a, const T& b) const
            {
                // Sort invalid items to the back.
                if (!a.IsValid())
                {
                    return false;
                }
                if (!b.IsValid())
                {
                    return true;
                }
                return GetKey(a) < GetKey(b);
            }
        };

        struct DefaultProjection
        {
            constexpr const T& operator()(const T& item) const
            {
                return item;
            }
        };

        struct SortItemsByKey
        {
            constexpr bool operator()(const T& a, const T& b) const
            {
                return GetKey(a) < GetKey(b);
            }
            constexpr bool operator()(const TKey& a, const T& b) const
            {
                return a < GetKey(b);
            }
            constexpr bool operator()(const T& a, const TKey& b) const
            {
                return GetKey(a) < b;
            }
            constexpr bool operator()(const TKey& a, const TKey& b) const
            {
                return a < b;
            }
        };

        struct CompareItemsByKey
        {
            constexpr bool operator()(const T& a, const T& b) const
            {
                return GetKey(a) == GetKey(b);
            }
        };

        // Find the index of the first item that matches a given key.
        // Searches the sorted range first, followed by the unsorted range.
        // Optionally provide a subset of the sorted range to search within.
        // In the best case this operation is O(log N). In the worst case it is O(N).
        template <class TKeyEquals = std::equal_to<TKey>>
        uint32 FindIndexOfFirstSubItemWithKey(
            const TKey& key,
            const TKeyEquals& keyEquals,
            const SortedItemRun& run = {}) const
        {
            uint32 subIndex = 0;
            return FindIndexOfSubItemWithKey(key, subIndex, keyEquals, run);
        }

        // Find the index of the Nth item that matches a given key.
        // Searches the sorted range first, followed by the unsorted range.
        // Optionally provide a subset of the sorted range to search within.
        // In the best case this operation is O(log N). In the worst case it is O(N).
        template <class TKeyEquals = std::equal_to<TKey>>
        uint32 FindIndexOfSubItemWithKey(
            const TKey& key,
            uint32& subIndex,
            const TKeyEquals& keyEquals,
            const SortedItemRun& run = {}) const
        {
            if (Storage.IsEmpty())
            {
                return Index<uint32>::None;
            }

            // First try to search the sorted range since it will be O(log N).
            uint32 index = FindIndexOfSubItemWithKeyInSortedRange(key, subIndex, run, keyEquals);
            if (index != Index<uint32>::None)
            {
                return index;
            }

            // Fallback to searching the unsorted range which is O(N).
            return FindIndexOfSubItemWithKeyInUnsortedRange(key, subIndex, keyEquals);
        }

        // Find the index of the Nth item that matches a given key in the sorted range.
        // Optionally provide a subset of the sorted range to search within.
        // In the best case this operation is O(log N).
        template <class TKeyEquals = std::equal_to<TKey>>
        uint32 FindIndexOfSubItemWithKeyInSortedRange(
            const TKey& key,
            uint32& subIndex,
            const SortedItemRun& run,
            const TKeyEquals& keyEquals) const
        {
            if (SortedNum == 0)
            {
                return Index<uint32>::None;
            }

            // If a valid run was specified, try to use that because it should greatly reduce the search area.
            // We can't actually trust that the run provided will contain the item...
            if (run.Start < run.End && run.End < SortedNum)
            {
                // Check if the provided run can possibly contain the item.
                // If the key is less than the key of the item at the start of the run, then the run doesn't contain the item.
                if (key < GetKey(Storage[run.Start]))
                {
                    SortedItemRun beforeRun = { 0, run.Start };
                    return FindIndexOfSubItemWithKeyInSortedRun(key, subIndex, beforeRun, keyEquals);
                }

                // Check if the provided run can possibly contain the item.
                // If the key is greater than the key of the item at the end of the run, then the run doesn't contain the item.
                if (key > GetKey(Storage[run.End]))
                {
                    SortedItemRun afterRun = { run.End, SortedNum };
                    return FindIndexOfSubItemWithKeyInSortedRun(key, subIndex, afterRun, keyEquals);
                }

                // Now check the run itself.
                return FindIndexOfSubItemWithKeyInSortedRun(key, subIndex, run, keyEquals);
            }

            // If no run was specified then just search the entire sorted range.
            return FindIndexOfSubItemWithKeyInSortedRange(key, subIndex, keyEquals);
        }

        // Find the index of the Nth item that matches a given key in a subset of the sorted range.
        // In the best case this operation is O(log N).
        template <class TKeyEquals = std::equal_to<TKey>>
        uint32 FindIndexOfSubItemWithKeyInSortedRun(
            const TKey& key,
            uint32& subIndex,
            const SortedItemRun& run,
            const TKeyEquals& keyEquals) const
        {
            if (run.Start >= run.End || run.End >= SortedNum)
            {
                // Invalid range.
                return Index<uint32>::None;
            }
            TItemsConstIter begin = Storage.begin() + run.Start;
            TItemsConstIter end = Storage.begin() + run.End;
            return FindIndexOfSubItemWithKeyInSortedRange(begin, end, key, subIndex, keyEquals);
        }

        // Find the index of the first item that matches a given key in the entire sorted range.
        // In the best case this operation is O(log N).
        template <class TKeyEquals = std::equal_to<TKey>>
        uint32 FindIndexOfFirstSubItemWithKeyInSortedRange(const TKey& key, const TKeyEquals& keyEquals) const
        {
            uint32 subIndex = 0;
            return FindIndexOfSubItemWithKeyInSortedRange(key, subIndex, keyEquals);
        }

        // Find the index of the Nth item that matches a given key in the entire sorted range.
        // In the best case this operation is O(log N).
        template <class TKeyEquals = std::equal_to<TKey>>
        uint32 FindIndexOfSubItemWithKeyInSortedRange(const TKey& key, uint32& subIndex, const TKeyEquals& keyEquals) const
        {
            uint32 sortedNum = SortedNum;
            if (sortedNum == 0)
            {
                // Not sorted yet so there's nothing to search.
                return Index<uint32>::None;
            }
            TItemsConstIter begin = Storage.begin();
            TItemsConstIter end = begin + sortedNum;
            return FindIndexOfSubItemWithKeyInSortedRange(begin, end, key, subIndex, keyEquals);
        }

        // Find the index of the Nth item that matches a given key in a subset of the sorted range.
        // In the best case this operation is O(log N).
        template <class TKeyEquals = std::equal_to<TKey>>
        uint32 FindIndexOfSubItemWithKeyInSortedRange(
            const TItemsConstIter& begin,
            const TItemsConstIter& end,
            const TKey& key,
            uint32& subIndex,
            const TKeyEquals& keyEquals) const
        {
            if (begin == end)
            {
                return Index<uint32>::None;
            }

            // Find the first item that matches the key and search from there.
            TItemsConstIter iter = std::lower_bound(begin, end, key, SortItemsByKey());
            while (iter < end && keyEquals(GetKey(*iter), key))
            {
                if (iter->IsValid() && subIndex-- == 0)
                {
                    return static_cast<uint32>(iter - Storage.begin());
                }
                ++iter;
            }

            // We did not find the item within the sorted range.
            return Index<uint32>::None;
        }

        // Find the index of the first item that matches a given key in the entire unsorted range.
        // In the best case this operation is O(N).
        template <class TKeyEquals = std::equal_to<TKey>>
        uint32 FindIndexOfFirstSubItemWithKeyInUnsortedRange(const TKey& key, const TKeyEquals& keyEquals) const
        {
            uint32 subIndex = 0;
            return FindIndexOfSubItemWithKeyInUnsortedRange(key, subIndex, keyEquals);
        }

        // Find the index of the Nth item that matches a given key in the entire unsorted range.
        // In the best case this operation is O(N).
        template <class TKeyEquals = std::equal_to<TKey>>
        uint32 FindIndexOfSubItemWithKeyInUnsortedRange(
            const TKey& key,
            uint32& subIndex,
            const TKeyEquals& keyEquals) const
        {
            TItemsConstIter begin = Storage.begin();
            TItemsConstIter end = Storage.end();
            TItemsConstIter sortedEnd = begin + SortedNum;

            // Starting at the end of the sorted range, find the Nth item that matches the key.
            TItemsConstIter iter = sortedEnd;
            while (iter < end)
            {
                const T& item = *iter;
                if (keyEquals(GetKey(item), key) && item.IsValid() && subIndex-- == 0)
                {
                    return static_cast<uint32>(iter - begin);
                }
                ++iter;
            }

            // We did not find the item within the unsorted range.
            return Index<uint32>::None;
        }

        // Find the index of the next item that matches a given key after a given index.
        template <class TKeyEquals = std::equal_to<TKey>>
        uint32 FindIndexOfNextItemWithKey(const TKey& key, uint32 currIndex, const TKeyEquals& keyEquals = {}) const
        {
            if (Storage.IsEmpty())
            {
                return Index<uint32>::None;
            }

            uint32 index = currIndex + 1;

            // Search the sorted section
            while (index < SortedNum)
            {
                if (!keyEquals(GetKey(Storage[index]), key))
                {
                    // No longer in the sorted run matching the key, so we can stop searching the sorted section.
                    // Skip to the unsorted section by setting the index to SortedNum.
                    index = SortedNum;
                    break;
                }

                const T& item = Storage[index];
                if (item.IsValid())
                {
                    return index;
                }

                ++index;
            }

            // Search the unsorted section
            PHX_ASSERT(index >= SortedNum);
            while (index < Storage.GetNum())
            {
                const T& item = Storage[index];
                if (keyEquals(GetKey(item), key) && item.IsValid())
                {
                    return index;
                }
                ++index;
            }

            return Index<uint32>::None;
        }

        // Find the index of an item in the list.
        template <class TItemEquals = std::equal_to<T>, class TKeyEquals = std::equal_to<TKey>>
        uint32 FindIndexOfItem(
            const T& item,
            const TItemEquals& itemEquals,
            const TKeyEquals& keyEquals) const
        {
            if (Storage.IsEmpty())
            {
                return Index<uint32>::None;
            }

            TItemsConstIter begin = Storage.begin();
            TItemsConstIter end = Storage.end();
            TItemsConstIter sortedEnd = begin + SortedNum;

            PHX_ASSERT(sortedEnd <= end);

            TKey itemKey = GetKey(item);

            // Search the sorted section
            if (sortedEnd != begin)
            {
                uint32 index = FindIndexOfFirstSubItemWithKeyInSortedRange(itemKey, keyEquals);
                while (index < SortedNum && keyEquals(GetKey(Storage[index]), itemKey))
                {
                    if (Storage[index].IsValid() && itemEquals(item, Storage[index]))
                    {
                        return index;
                    }
                    ++index;
                }
            }

            // Search the unsorted section
            TItemsConstIter iter = sortedEnd;
            while (iter < end)
            {
                if (keyEquals(GetKey(*iter), itemKey) && iter->IsValid() && itemEquals(item, *iter))
                {
                    return static_cast<uint32>(iter - begin);
                }
                ++iter;
            }

            return Index<uint32>::None;
        }

        // Find the Nth item that matches a given key.
        template <class TKeyEquals = std::equal_to<TKey>>
        T* FindSubItemInternal(const TKey& key, uint32 subIndex, uint32& outIndex, const TKeyEquals& keyEquals)
        {
            if (IsEmpty())
            {
                return nullptr;
            }
            outIndex = FindIndexOfSubItemWithKey(key, subIndex, keyEquals);
            return outIndex != Index<uint32>::None ? &Storage[outIndex] : nullptr;
        }

        // Find the Nth item that matches a given key.
        template <class TKeyEquals = std::equal_to<TKey>>
        const T* FindSubItemInternal(const TKey& key, uint32 subIndex, uint32& outIndex, const TKeyEquals& keyEquals) const
        {
            if (IsEmpty())
            {
                return nullptr;
            }
            outIndex = FindIndexOfSubItemWithKey(key, subIndex, keyEquals);
            return outIndex != Index<uint32>::None ? &Storage[outIndex] : nullptr;
        }

        // Find the next item that matches a given key after a given index.
        template <class TKeyEquals = std::equal_to<TKey>>
        T* FindNextItemInternal(const TKey& key, uint32 currIndex, uint32& outIndex, const TKeyEquals& keyEquals)
        {
            outIndex = FindIndexOfNextItemWithKey(key, currIndex, keyEquals);
            return outIndex != Index<uint32>::None ? &Storage[outIndex] : nullptr;
        }

        // Find the next item that matches a given key after a given index.
        template <class TKeyEquals = std::equal_to<TKey>>
        const T* FindNextItemInternal(const TKey& key, uint32 currIndex, uint32& outIndex, const TKeyEquals& keyEquals) const
        {
            if (IsEmpty())
            {
                return nullptr;
            }
            outIndex = FindIndexOfNextItemWithKey(key, currIndex, keyEquals);
            return outIndex != Index<uint32>::None ? &Storage[outIndex] : nullptr;
        }

        // Shift items towards the back starting at the given index to make room for a new item.
        bool ShiftItemsTowardsBack(uint32 index)
        {
            // Find the next invalid item slot.
            uint32 i = index;
            for (; i < Storage.GetNum(); ++i)
            {
                if (!Storage[i].IsValid())
                {
                    break;
                }
            }

            // Make room for a new item if there was no invalid item slot.
            if (i == Storage.GetNum() && !Storage.PushBack())
            {
                // Queue is full, can't add any new items.
                return false;
            }

            // Shift all items back towards the invalid slot.
            uint32 j = i;
            for (; j > index; --j)
            {
                Storage[j] = Storage[j - 1];
            }

            PHX_ASSERT(j == index);
            return true;
        }

        // Insert an item at the given index possibly shifting items back to make room.
        bool InsertInternal(uint32 index, const T& item)
        {
            if (Storage.IsFull())
            {
                return false;
            }

            // Treat insert at Size-1 as a push-back.
            if (index > Storage.GetNum())
            {
                return false;
            }

            // Only allow sorted inserts in the sorted section.
            // Otherwise, queries will break until the list is sorted again.
            if (index <= SortedNum && GetKey(item) > GetKey(Storage[index]))
            {
                return false;
            }

            if (!ShiftItemsTowardsBack(index))
            {
                return false;
            }

            bool wasValid = Storage[index].IsValid();

            Storage[index] = item;

            if (!wasValid && Storage[index].IsValid())
            {
                ++NumValidItems;
            }
            else if (wasValid && !Storage[index].IsValid())
            {
                --NumValidItems;
            }

            // Item is being inserted into the sorted section so increase the number of sorted items.
            if (index <= SortedNum)
            {
                ++SortedNum;
            }

            return true;
        }

        template <class ...TArgs>
        bool EmplaceInsertInternal(uint32 index, TArgs&&... args)
        {
            if (Storage.IsFull())
            {
                return false;
            }

            // Treat insert at Size-1 as a push-back.
            if (index > Storage.GetSize())
            {
                return false;
            }

            // Only allow sorted inserts in the sorted section.
            // Otherwise, queries will break until the list is sorted again.
            if (index <= SortedNum)
            {
                T test{std::forward<TArgs>(args)...};
                if (GetKey(test) > GetKey(Storage[index]))
                {
                    return false;
                }
            }

            if (!ShiftItemsTowardsBack(index))
            {
                return false;
            }

            bool wasValid = Storage[index].IsValid();

            new (&Storage[index]) T(std::forward<TArgs>(args)...);

            if (!wasValid && Storage[index].IsValid())
            {
                ++NumValidItems;
            }
            else if (wasValid && !Storage[index].IsValid())
            {
                --NumValidItems;
            }

            // Item is being inserted into the sorted section so increase the number of sorted items.
            if (index <= SortedNum)
            {
                ++SortedNum;
            }

            return true;
        }
    };

    template <class T, class TGetKey, class TStoragePolicy>
    const TGetKey TSortedList<T, TGetKey, TStoragePolicy>::GetKey = {};

    template <class T, class TGetKey, uint32 N>
    using TInlineSortedList = TSortedList<T, TGetKey, InlineStoragePolicy<N>>;

    template <class T, class TGetKey>
    using TFixedSortedList = TSortedList<T, TGetKey, FixedStoragePolicy>;

    template <class T, class TGetKey>
    using THeapSortedList = TSortedList<T, TGetKey, HeapStoragePolicy>;
}
