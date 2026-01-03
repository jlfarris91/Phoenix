#pragma once

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Random.h"
#include "PhoenixSim/Containers/FixedArray.h"
#include "PhoenixSim/FixedPoint/FixedMath.h"

namespace Phoenix
{
    template <class T, uint32 N, class TWeight = Value, class TRandom = Random>
    struct TFixedWeightedSet
    {
        static constexpr uint32 Capacity = N;

        struct WeightedItem
        {
            T Value;
            TWeight Weight;
        };

        using TItems = TFixedArray<WeightedItem, Capacity>;

        TFixedWeightedSet() = default;

        TFixedWeightedSet(std::initializer_list<WeightedItem> list)
        {
            for (const auto& elem : list)
            {
                EmplaceBack(elem);
            }
        }

        uint32 Num() const
        {
            return Items.Num();
        }

        bool IsEmpty() const
        {
            return Items.IsEmpty();
        }

        bool IsFull() const
        {
            return Items.IsFull();
        }

        TWeight GetTotalWeight() const
        {
            return TotalWeight;
        }

        bool IsValidIndex(uint32 index) const
        {
            return Items.IsValidIndex(index);
        }

        bool PushBack(const T& value, TWeight weight)
        {
            if (Items.PushBack({ value, weight }))
            {
                TotalWeight += weight;
                return true;
            }
            return false;
        }

        bool Add(const T& value, TWeight weight)
        {
            return PushBack(value, weight);
        }

        template <class ...TArgs>
        bool EmplaceBack(TArgs&&... args)
        {
            if (Items.EmplaceBack(std::forward<TArgs>(args)...))
            {
                TotalWeight += Items.Back().Weight;
                return true;
            }
            return false;
        }

        bool Remove(const T& value)
        {
            uint32 numRemoved = 0;
            for (uint32 i = 0; i < Items.Num();)
            {
                if (Items[i].Value == value)
                {
                    TotalWeight -= Items[i].Weight;
                    Items.RemoveAtUnsorted(i);
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

            TotalWeight -= Items[index].Weight;
            Items.RemoveAtUnsorted(index);

            return true;
        }

        TWeight GetWeight(uint32 index) const
        {
            return IsValidIndex(index) ? Items[index].Weight : TWeight{};
        }

        bool SetWeight(uint32 index, TWeight weight)
        {
            if (!IsValidIndex(index))
            {
                return false;
            }

            TotalWeight -= Items[index].Weight;
            Items[index].Weight = weight;
            TotalWeight += weight;

            return true;
        }

        Value GetItemChance(uint32 index) const
        {
            if (!IsValidIndex(index))
            {
                return {};
            }
            return TotalWeight == 0 ? 1 : Items[index].Weight / TotalWeight;
        }

        uint32 SampleIndex(TWeight weight) const
        {
            if (IsEmpty())
            {
                return Index<uint32>::None;
            }

            weight = Clamp<TWeight>(weight, 0, TotalWeight);

            uint32 i = 0;
            for (; i < Items.Num() - 1; ++i)
            {
                if (weight >= 0 && weight <= Items[i].Weight)
                {
                    break;
                }
                weight -= Items[i].Weight;
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
            outValue = Items[index].Value;
            return true;
        }

        bool SampleAndRemove(TWeight weight, T& outValue) const
        {
            uint32 index = SampleIndex(weight);
            if (!IsValidIndex(index))
            {
                return false;
            }
            outValue = Items[index].Value;
            TotalWeight -= Items[index].Weight;
            Items.RemoveAtUnsorted(index);
            return true;
        }

        bool GetRandom(TRandom& random, T& outValue) const
        {
            TWeight weight = random.template RandomRange<TWeight>(0.0, TotalWeight);
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
            TFixedWeightedSet clone = *this;
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
            TWeight weight = random.template RandomRange<TWeight>(0.0, TotalWeight);
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
            Items.Reset();
            TotalWeight = {};
        }

        const WeightedItem& operator[](uint32 index) const
        {
            return Items[index];
        }

        typename TItems::ConstIter begin() const
        {
            return Items.begin();
        }

        typename TItems::ConstIter end() const
        {
            return Items.end();
        }

    private:

        TFixedArray<WeightedItem, Capacity> Items;
        TWeight TotalWeight = {};
    };
}
