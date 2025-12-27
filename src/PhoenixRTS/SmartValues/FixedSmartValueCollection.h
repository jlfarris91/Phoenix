
#pragma once

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Containers/FixedArray.h"

#include "PhoenixRTS/SmartValues/SmartValueAffector.h"

namespace Phoenix::RTS
{
    enum class ESmartValueType : uint8
    {
        Invalid,
        Base,
        Scale,
        Absolute
    };

    struct PHOENIX_RTS_API SmartValueHandle
    {
        static const SmartValueHandle Invalid;

        constexpr SmartValueHandle() : Id(0) {}
        constexpr SmartValueHandle(uint32 raw) : Id(raw) {}

        operator uint32() const;
        SmartValueHandle& operator=(const uint32& id);

    private:
        uint32 Id;
    };

    struct PHOENIX_RTS_API SmartValueAffectorHandle
    {
        static const SmartValueAffectorHandle Invalid;

        constexpr SmartValueAffectorHandle() : Id(0) {}
        constexpr SmartValueAffectorHandle(uint64 raw) : Id(raw) {}

        operator uint64() const;
        SmartValueAffectorHandle& operator=(const uint64& id);

    private:
        uint64 Id;
    };

    template <uint32 N>
    struct TFixedSmartValueCollection
    {
        static constexpr uint32 Capacity = N;
        static constexpr uint64 ValueIdMask = 0xFFFFFFFF00000000;
        static constexpr uint64 AffectorIdMask = 0x00000000FFFFFFFF;

        struct SmartValue
        {
            uint64 Id = 0;
            ESmartValueType Type = ESmartValueType::Base;
            Value Value = {};
            Time Duration = 0;

            bool IsValid() const { return Type != ESmartValueType::Invalid; }
            void Invalidate() { Type = ESmartValueType::Invalid; }
        };

        Value ResolveValue(SmartValueHandle handle, const Value& defaultValue = {}) const
        {
            Value result;
            if (!TryResolveValue(handle, result))
            {
                result = defaultValue;
            }
            return result;
        }

        bool TryResolveValue(SmartValueHandle handle, Value& outValue) const
        {
            uint32 currIndex = 0;
            const SmartValue* head = GetSmartValue(handle, currIndex);
            if (!head)
            {
                return false;
            }

            Value value = head->Value;
            Value scale = 1.0;
            Value absolute = 0.0;

            while (const SmartValue* affector = GetNextValueAffector(currIndex, currIndex))
            {
                switch (affector->Type)
                {
                    case ESmartValueType::Base:
                        {
                            value = affector->Value;
                            break;
                        }
                    case ESmartValueType::Scale:
                        {
                            scale *= affector->Value;
                            break;
                        }
                    case ESmartValueType::Absolute:
                        {
                            absolute = affector->Value;
                            break;
                        }
                    default: break;
                }
            }

            outValue = value * scale + absolute;
            return true;
        }

        // Acquires a new smart value.
        SmartValueHandle Acquire(Value baseValue = {})
        {
            if (Values.IsFull())
            {
                return SmartValueHandle::Invalid;
            }

            SmartValueHandle handle((uint64)++ValueIdGen << 32);

            SmartValue& value = Values.AddDefaulted_GetRef();
            value.Id = handle;
            value.Type = ESmartValueType::Base;
            value.Value = baseValue;
            value.Duration = 0;
            ++NumValidValues;

            return handle;
        }

        // Releases a smart value and all of its affectors.
        uint32 Release(SmartValueHandle& handle)
        {
            uint32 numRemoved = 0;

            auto begin = Values.begin();
            auto end = Values.end();
            auto sortedEnd = begin + SortedNum;

            // Search the sorted section
            if (sortedEnd != begin)
            {
                auto iter = std::lower_bound(begin, sortedEnd, handle << 32, SortedMaskedValueIdPred{ValueIdMask});
                while (iter != sortedEnd && (iter->Id & ValueIdMask) == handle)
                {
                    if (iter->IsValid())
                    {
                        iter->Invalidate();
                        ++numRemoved;
                        --NumValidValues;
                    }
                    ++iter;
                }
            }

            // Search the unsorted section
            auto iter = sortedEnd;
            while (iter != end)
            {
                if ((iter->Id & ValueIdMask) == handle && iter->IsValid())
                {
                    iter->Invalidate();
                    ++numRemoved;
                    --NumValidValues;
                }
                ++iter;
            }

            handle = SmartValueHandle::Invalid;
            return numRemoved;
        }

        SmartValueAffectorHandle AcquireAffector(
            SmartValueHandle handle,
            const FName& name,
            ESmartValueType type,
            Value value,
            Time duration = 0)
        {
            if (Values.IsFull())
            {
                return SmartValueAffectorHandle::Invalid;
            }

            SmartValueAffectorHandle affectorHandle(((uint64)handle << 32) | (uint32)name);

            SmartValue& affector = Values.AddDefaulted_GetRef();
            affector.Id = affectorHandle;
            affector.Type = type;
            affector.Value = value;
            affector.Duration = duration;
            ++NumValidValues;

            return affectorHandle;
        }

        bool ReleaseAffector(SmartValueAffectorHandle& handle)
        {
            uint32 index = FindIndexOfValue(handle);
            if (!index)
            {
                return false;
            }

            Values[index].Invalidate();
            --NumValidValues;

            handle = SmartValueAffectorHandle::Invalid;
            return true;
        }

        const SmartValue* GetSmartValue(SmartValueHandle handle, uint32& outIndex) const
        {
            auto begin = Values.begin();
            auto end = Values.end();
            auto sortedEnd = begin + SortedNum;

            uint64 id = handle << 32;

            // Search the sorted section
            if (sortedEnd != begin)
            {
                auto iter = std::lower_bound(begin, sortedEnd, id, SortedMaskedValueIdPred{ValueIdMask});
                if (iter != sortedEnd && iter->Id == id && iter->IsValid())
                {
                    outIndex = static_cast<uint32>(iter - begin);
                    return &*iter;
                }
            }

            // Search the unsorted section
            auto iter = sortedEnd;
            while (iter != end)
            {
                if (iter->Id == id && iter->IsValid())
                {
                    outIndex = static_cast<uint32>(iter - begin);
                    return &*iter;
                }
                ++iter;
            }

            return nullptr;
        }

        const SmartValue* GetNextValueAffector(SmartValueHandle handle, uint32 currIndex, uint32& outIndex) const
        {
            uint32 index = currIndex + 1;

            // Search the sorted section
            while (index < SortedNum && (Values[index]->Id & ValueIdMask) == handle)
            {
                const SmartValue& value = Values[index];
                if (value.IsValid())
                {
                    outIndex = index;
                    return &value;
                }
                ++index;
            }

            // Search the unsorted section
            index = SortedNum;
            while (index < Values.Num())
            {
                const SmartValue& value = Values[index];
                if ((Values[index]->Id & ValueIdMask) == handle && value.IsValid())
                {
                    outIndex = index;
                    return &value;
                }
                ++index;
            }

            outIndex = Index<uint32>::None;
            return nullptr;
        }

        void Tick(Time dt)
        {
            
        }

    private:

        struct SortedMaskedValueIdPred
        {
            hash64_t Mask;
            constexpr bool operator()(const SmartValue& value, hash64_t id) const
            {
                return (value.Id & Mask) < id;
            }
        };

        uint32 FindIndexOfValue(uint64 id, uint64 mask = -1)
        {
            auto begin = Values.begin();
            auto end = Values.end();
            auto sortedEnd = begin + SortedNum;

            // Search the sorted section
            if (sortedEnd != begin)
            {
                auto iter = std::lower_bound(begin, sortedEnd, id & mask, SortedMaskedValueIdPred{mask});
                while (iter != sortedEnd && iter->Id == id)
                {
                    if (iter->IsValid() && orderIndex-- == 0)
                    {
                        return iter - begin;
                    }
                    ++iter;
                }
            }

            // Search the unsorted section
            auto iter = sortedEnd;
            while (iter != end)
            {
                if (iter->Id == entityId && iter->IsValid() && orderIndex-- == 0)
                {
                    return iter - begin;
                }
                ++iter;
            }

            return Index<uint32>::None;
        }

        TFixedArray<SmartValue, Capacity> Values;
        uint32 ValueIdGen = 0;
        uint32 SortedNum = 0;
        uint32 NumValidValues = 0;
    };
}
