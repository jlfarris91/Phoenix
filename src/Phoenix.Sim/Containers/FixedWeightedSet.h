#pragma once

#include "FixedMemory.h"
#include "Phoenix/Platform.h"
#include "Phoenix/Random.h"
#include "Phoenix.Sim/Containers/FixedArray.h"
#include "Phoenix/FixedPoint/FixedMath.h"

namespace Phoenix
{
    template <class T, class TWeight = Value>
    struct WeightedItem
    {
        T Value;
        TWeight Weight;
    };

    template <class T, class TWeight, class TStoragePolicy>
    class TWeightedSetBase
    {
    protected:
        using TItem = WeightedItem<T, TWeight>;
        using TStorage = TArray<TItem, TStoragePolicy>;
        TStorage Storage;
        TWeight TotalWeight = {};
    };

    template <class T, class TWeight>
    class TWeightedSetBase<T, TWeight, FixedStoragePolicy>
    {
    public:

        PHX_DECLARE_BLOCK_CONTAINER(TWeightedSetBase)
        {
            uint32 Capacity;
        };

    protected:
        using TItem = WeightedItem<T, TWeight>;
        using TStorage = TFixedArray<TItem>;
        TStorage Storage;
        TWeight TotalWeight = {};
    };

    template <class T, class TWeight>
    void TWeightedSetBase<T, TWeight, FixedStoragePolicy>::Construct(BlockBufferAllocator& allocator, const Config& config)
    {
        Storage.Construct(allocator, config.Capacity);
    }

    template <class T, class TWeight>
    BlockBufferLayout TWeightedSetBase<T, TWeight, FixedStoragePolicy>::StaticLayout(const Config& config)
    {
        return BlockBufferLayout::For<TWeightedSetBase>().template Container<TStorage>(config.Capacity);
    }

    template <class T, class TWeight, class TRandom, class TStoragePolicy>
    class TWeightedSet : public TWeightedSetBase<T, TWeight, TStoragePolicy>
    {
        using Super = TWeightedSetBase<T, TWeight, TStoragePolicy>;
        using Super::Super;
        using Super::Storage;
        using Super::TotalWeight;

        using TStorage = typename Super::TStorage;
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
            return Storage.GetNum();
        }

        bool IsEmpty() const
        {
            return Storage.IsEmpty();
        }

        bool IsFull() const
        {
            return Storage.IsFull();
        }

        TWeight GetTotalWeight() const
        {
            return TotalWeight;
        }

        bool IsValidIndex(uint32 index) const
        {
            return Storage.IsValidIndex(index);
        }

        bool PushBack(const TItem& item)
        {
            if (Storage.PushBack(item))
            {
                TotalWeight += item.Weight;
                return true;
            }
            return false;
        }

        template <class ...TArgs>
        bool EmplaceBack(TArgs&&... args)
        {
            if (Storage.EmplaceBack(std::forward<TArgs>(args)...))
            {
                TotalWeight += Storage.Back().Weight;
                return true;
            }
            return false;
        }

        bool Remove(const T& value)
        {
            uint32 numRemoved = 0;
            for (uint32 i = 0; i < Storage.GetNum();)
            {
                if (Storage[i].Value == value)
                {
                    TotalWeight -= Storage[i].Weight;
                    Storage.RemoveAtUnsorted(i);
                    ++numRemoved;
                }
                else
                {
                    ++i;
                }
            }
            return numRemoved;
        }

        bool RemoveAt(uint32 index)
        {
            if (!IsValidIndex(index))
            {
                return false;
            }

            TotalWeight -= Storage[index].Weight;
            Storage.RemoveAtUnsorted(index);

            return true;
        }

        TWeight GetWeight(uint32 index) const
        {
            return IsValidIndex(index) ? Storage[index].Weight : TWeight{};
        }

        bool SetWeight(uint32 index, TWeight weight)
        {
            if (!IsValidIndex(index))
            {
                return false;
            }

            TotalWeight -= Storage[index].Weight;
            Storage[index].Weight = weight;
            TotalWeight += weight;

            return true;
        }

        Value GetItemChance(uint32 index) const
        {
            if (!IsValidIndex(index))
            {
                return {};
            }
            return TotalWeight == 0 ? 1 : Storage[index].Weight / TotalWeight;
        }

        uint32 SampleIndex(TWeight weight) const
        {
            if (IsEmpty())
            {
                return Index<uint32>::None;
            }

            weight = Clamp<TWeight>(weight, 0, TotalWeight);

            uint32 i = 0;
            for (; i < Storage.GetNum() - 1; ++i)
            {
                if (weight >= 0 && weight <= Storage[i].Weight)
                {
                    break;
                }
                weight -= Storage[i].Weight;
            }

            return i;
        }

        bool Sample(TWeight weight, T& outValue) const
        {
            uint32 index = SampleIndex(weight);
            if (!IsValidIndex(index))
            {
                return false;
            }
            outValue = Storage[index].Value;
            return true;
        }

        bool SampleAndRemove(TWeight weight, T& outValue) const
        {
            uint32 index = SampleIndex(weight);
            if (!IsValidIndex(index))
            {
                return false;
            }
            outValue = Storage[index].Value;
            TotalWeight -= Storage[index].Weight;
            Storage.RemoveAtUnsorted(index);
            return true;
        }

        bool GetRandom(TRandom& random, T& outValue) const
        {
            TWeight weight = random.template Range<TWeight>(0.0, TotalWeight);
            return Sample(weight, outValue);
        }

        template <class TArray>
        uint32 GetRandom(TRandom& random, uint32 n, TArray& outValues) const
        {
            uint32 count = 0;
            for (uint32 i = 0; i < n; ++i)
            {
                T value;
                if (GetRandom(random, value))
                {
                    outValues.PushBack(value);
                    ++count;
                }
                else
                {
                    break;
                }
            }
            return count;
        }

        template <class TArray>
        uint32 GetRandomUnique(TRandom& random, uint32 n, TArray& outValues) const
        {
            TWeightedSet clone = *this;
            uint32 count = 0;
            for (uint32 i = 0; i < n; ++i)
            {
                T value;
                if (clone.PopRandom(random, value))
                {
                    outValues.PushBack(value);
                    ++count;
                }
                else
                {
                    break;
                }
            }
            return count;
        }

        bool PopRandom(TRandom& random, T& outValue) const
        {
            TWeight weight = random.template Range<TWeight>(0.0, TotalWeight);
            return SampleAndRemove(weight, outValue);
        }

        template <class TArray>
        uint32 PopRandom(TRandom& random, uint32 n, TArray& outValues) const
        {
            uint32 count = 0;
            for (uint32 i = 0; i < n; ++i)
            {
                T value;
                if (PopRandom(random, value))
                {
                    outValues.PushBack(value);
                    ++count;
                }
                else
                {
                    break;
                }
            }
            return count;
        }

        void Reset()
        {
            Storage.Reset();
            TotalWeight = {};
        }

        const TItem& operator[](uint32 index) const
        {
            return Storage[index];
        }

        typename TStorage::ConstIter begin() const
        {
            return Storage.begin();
        }

        typename TStorage::ConstIter end() const
        {
            return Storage.end();
        }
    };

    template <class T, uint32 N, class TWeight = Value, class TRandom = Random>
    using TInlineWeightedSet = TWeightedSet<T, TWeight, TRandom, InlineStoragePolicy<N>>;

    template <class T, class TWeight = Value, class TRandom = Random>
    using TFixedWeightedSet = TWeightedSet<T, TWeight, TRandom, FixedStoragePolicy>;
}
