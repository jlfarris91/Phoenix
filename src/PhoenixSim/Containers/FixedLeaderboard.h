#pragma once

#include "FixedArray.h"
#include "FixedMemory.h"

namespace Phoenix
{
    template <class T, class TRank>
    struct TLeaderboardItem
    {
        T Value;
        TRank Rank;
    };

    template <class T, class TRank, class TStoragePolicy>
    class TLeaderboardBase
    {
    protected:
        using TItem = TLeaderboardItem<T, TRank>;
        using TStorage = TArray<TItem, TStoragePolicy>;
        TStorage Storage;
    };

    template <class T, class TRank>
    class TLeaderboardBase<T, TRank, FixedStoragePolicy>
    {
    public:

        TLeaderboardBase() = default;

        template <class TAllocator>
        TLeaderboardBase(TAllocator& allocator, uint32 capacity)
            : Storage(allocator, capacity)
        {
        }

        template <class TAllocator, class TOtherStoragePolicy>
        TLeaderboardBase(TAllocator& allocator, uint32 capacity, const TLeaderboardBase<T, TRank, TOtherStoragePolicy>& other)
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
        using TItem = TLeaderboardItem<T, TRank>;
        using TStorage = TFixedArray<TItem>;
        TStorage Storage;
    };

    template <class T, class TRank, class TStoragePolicy>
    class TLeaderboard : TLeaderboardBase<T, TRank, TStoragePolicy>
    {
        using Super = TLeaderboardBase<T, TRank, TStoragePolicy>;
        using TItem = typename Super::TItem;
        using Super::Super;
        using Super::Storage;

    public:

        PHX_FORCEINLINE uint32 GetCapacity() const
        {
            return Storage.GetCapacity();
        }

        PHX_FORCEINLINE bool IsFull() const
        {
            return Storage.IsFull();
        }

        PHX_FORCEINLINE bool IsEmpty() const
        {
            return Storage.IsEmpty();
        }

        PHX_FORCEINLINE uint32 GetNum() const
        {
            return Storage.GetNum();
        }

        void Reset()
        {
            Storage.Reset();
        }

        bool Add(const T& value, const TRank& rank)
        {
            uint32 i = 0;
            for (; i < Storage.GetNum(); ++i)
            {
                TItem& item = Storage[i];
                if (item.Rank > rank)
                {
                    break;
                }
            }

            if (i == Storage.GetNum())
            {
                if (Storage.IsFull())
                {
                    return false;
                }

                Storage.PushBack({ value, rank });
                return true;
            }

            return Storage.Insert({ value, rank }, i);
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
    };

    template <class T, class TRank, uint32 N>
    using TInlineLeaderboard = TLeaderboard<T, TRank, InlineStoragePolicy<N>>;

    template <class T, class TRank>
    using TFixedLeaderboard = TLeaderboard<T, TRank, FixedStoragePolicy>;
}
