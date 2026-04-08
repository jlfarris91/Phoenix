#pragma once

#include "PhoenixSim/Name.h"
#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Containers/FixedBuffer.h"
#include "PhoenixSim/Containers/FixedMap.h"

namespace Phoenix
{
    template <class TBufferStoragePolicy, class TMapStoragePolicy>
    class TStringTableBase
    {
    protected:
        TBuffer<TBufferStoragePolicy> Buffer;
        TMap<FName, uint32, TMapStoragePolicy> NameToIndex;
    };

    template <>
    class TStringTableBase<FixedStoragePolicy, FixedStoragePolicy>
    {
    public:
        struct Config
        {
            uint32 MaxNumStrings = 0;
            uint32 BufferCapacity = 0;
        };

        TStringTableBase() = default;

        template <class TAllocator>
        TStringTableBase(TAllocator& allocator, const Config& config)
            : Configuration(config)
            , Buffer(allocator, config.BufferCapacity)
            , NameToIndex(allocator, config.MaxNumStrings)
        {
        }

        template <class TAllocator, class TOtherBufferStoragePolicy, class TOtherMapStoragePolicy>
        TStringTableBase(TAllocator& allocator, const Config& config, const TStringTableBase<TOtherBufferStoragePolicy, TOtherMapStoragePolicy>& other)
            : Configuration(config)
            , Buffer(allocator, config.BufferCapacity, other.Buffer)
            , NameToIndex(allocator, config.MaxNumStrings, other.NameToIndex)
        {
        }

        PHX_FORCEINLINE const Config& GetConfig() const
        {
            return Configuration;
        }

        PHX_FORCEINLINE static uint32 GetAllocSizeBytes(const Config& config)
        {
            uint32 allocSize = 0;
            allocSize += TFixedBuffer::GetAllocSizeBytes(config.BufferCapacity);
            allocSize += TFixedMap<FName, uint32>::GetAllocSizeBytes(config.MaxNumStrings);
            return allocSize;
        }

        PHX_FORCEINLINE uint32 GetAllocSizeBytes() const
        {
            return GetAllocSizeBytes(Configuration);
        }

    protected:
        Config Configuration;
        TFixedBuffer Buffer;
        TFixedMap<FName, uint32> NameToIndex;
    };

    template <class TBufferStoragePolicy, class TMapStoragePolicy>
    class PHOENIX_SIM_API TStringTable : public TStringTableBase<TBufferStoragePolicy, TMapStoragePolicy>
    {
        using Super = TStringTableBase<TBufferStoragePolicy, TMapStoragePolicy>;
        using Super::Super;
        using Super::Buffer;
        using Super::NameToIndex;

    public:

        bool IsFull() const
        {
            return Buffer.IsFull() || NameToIndex.IsFull();
        }

        bool IsEmpty() const
        {
            return NameToIndex.IsEmpty();
        }

        bool Contains(const FName& name) const
        {
            return NameToIndex.Contains(name);
        }

        const char* Get(const FName& name) const
        {
            const uint32* offsetPtr = NameToIndex.GetPtr(name);
            return offsetPtr ? reinterpret_cast<const char*>(Buffer.GetData() + *offsetPtr) : nullptr;
        }

        const char* Store(const char* str, uint32 len)
        {
            return StoreAs(str, len, FName(str, len));
        }

        const char* StoreAs(const char* str, uint32 len, const FName& name)
        {
            if (IsFull())
            {
                return nullptr;
            }

            if (const char* existing = Get(name))
            {
                return existing;
            }

            if (!Buffer.CanWrite(len + 1))
            {
                return nullptr;
            }

            uint32 offset = Buffer.GetSize();
            Buffer.Append(str, len);
            Buffer.template Write<uint8>(0); // Null terminator

            NameToIndex.Insert(name, offset);
            return reinterpret_cast<const char*>(Buffer.GetData() + offset);
        }
    };

    template <uint32 MaxStrings, uint32 BufferCapacity>
    using InlineStringTable = TStringTable<InlineStoragePolicy<MaxStrings>, InlineStoragePolicy<BufferCapacity>>;

    using FixedStringTable = TStringTable<FixedStoragePolicy, FixedStoragePolicy>;

    using HeapStringTable = TStringTable<HeapStoragePolicy, HeapStoragePolicy>;
}
