#pragma once

#include "Phoenix.Sim/Name.h"
#include "Phoenix.Sim/Platform.h"
#include "Phoenix.Sim/Containers/FixedBuffer.h"
#include "Phoenix.Sim/Containers/FixedMap.h"

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

        PHX_DECLARE_BLOCK_CONTAINER(TStringTableBase)
        {
            uint32 MaxNumStrings = 0;
            uint32 BufferCapacity = 0;
        };

    protected:
        TFixedBuffer Buffer;
        TFixedMap<FName, uint32> NameToIndex;
    };

    inline void TStringTableBase<FixedStoragePolicy, FixedStoragePolicy>::Construct(BlockBufferAllocator& allocator, const Config& config)
    {
        Buffer.Construct(allocator, config.BufferCapacity);
        NameToIndex.Construct(allocator, config.MaxNumStrings);
    }

    inline BlockBufferLayout TStringTableBase<FixedStoragePolicy, FixedStoragePolicy>::StaticLayout(const Config& config)
    {
        return BlockBufferLayout::For<TStringTableBase>()
            .Container<TFixedBuffer>("Buffer", config.BufferCapacity)
            .Container<TFixedMap<FName, uint32>>("NameToIndex", config.MaxNumStrings);
    }

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
            Buffer.Write(str, len);
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
