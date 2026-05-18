
#pragma once

#include <algorithm>
#include <cstring>

#include "FixedMemory.h"
#include "PhoenixSim/Platform.h"

namespace Phoenix
{
    template <class TStoragePolicy>
    class TBufferBase
    {
    protected:
        using TStorage = TContiguousStorage<uint8, TStoragePolicy>;
        TStorage Storage;
        uint32 WritePos = 0;
        uint32 ReadPos = 0;
        uint32 Size = 0;
    };

    template <>
    class TBufferBase<FixedStoragePolicy>
    {
    public:

        PHX_DECLARE_BLOCK_CONTAINER(TBufferBase)
        {
            uint32 Capacity;
        };

    protected:
        using TStorage = TFixedStorage<uint8>;
        TStorage Storage;
        uint32 WritePos = 0;
        uint32 ReadPos = 0;
        uint32 Size = 0;
    };

    inline void TBufferBase<FixedStoragePolicy>::Construct(BlockBufferAllocator& allocator, const Config& config)
    {
        Storage.Construct(allocator, config.Capacity);
    }

    inline BlockBufferLayout TBufferBase<FixedStoragePolicy>::StaticLayout(const Config& config)
    {
        return BlockBufferLayout::For<TBufferBase>().Container<TStorage>(config.Capacity);
    }

    template <class TStoragePolicy>
    class PHOENIX_SIM_API TBuffer : public TBufferBase<TStoragePolicy>
    {
        using Super = TBufferBase<TStoragePolicy>;
        using Super::Super;
        using Super::Storage;
        using Super::WritePos;
        using Super::ReadPos;
        using Super::Size;

    public:

        enum class ESeekOffset
        {
            Begin,
            End,
            Relative
        };

        PHX_FORCEINLINE uint32 GetCapacity() const
        {
            return Storage.GetCapacity();
        }

        PHX_FORCEINLINE uint8* GetData()
        {
            return Storage.GetData();
        }

        PHX_FORCEINLINE const uint8* GetData() const
        {
            return Storage.GetData();
        }

        uint32 GetSize() const
        {
            return Size;
        }

        bool IsEmpty() const
        {
            return WritePos == 0;
        }

        bool IsFull() const
        {
            return WritePos == GetCapacity();
        }

        bool CanWrite(uint32 size = 0) const
        {
            return WritePos + size <= GetCapacity();
        }

        bool CanRead(uint32 size = 0) const
        {
            return ReadPos + size <= Size;
        }

        void SeekWrite(int32 pos, ESeekOffset offset = ESeekOffset::Begin)
        {
            switch (offset)
            {
            case ESeekOffset::End:
                {
                    pos = static_cast<int32>(Size) - pos;
                }
                break;
            case ESeekOffset::Relative:
                {
                    pos += static_cast<int32>(WritePos);
                }
                break;
            case ESeekOffset::Begin:
                break;
            }
            WritePos = std::clamp(pos, 0, static_cast<int32>(GetCapacity()));
        }

        void SeekRead(int32 pos, ESeekOffset offset = ESeekOffset::Begin)
        {
            switch (offset)
            {
            case ESeekOffset::End:
                {
                    pos = static_cast<int32>(Size) - pos;
                }
                break;
            case ESeekOffset::Relative:
                {
                    pos += static_cast<int32>(ReadPos);
                }
                break;
            case ESeekOffset::Begin:
                break;
            }
            ReadPos = std::clamp(pos, 0, static_cast<int32>(GetCapacity()));
        }

        uint32 Write(const void* source, uint32 len)
        {
            len = std::max(GetCapacity() - WritePos, len);
            memcpy(Storage.GetData() + WritePos, source, len);
            WritePos += len;
            Size = std::max(Size, WritePos);
            return len;
        }

        template <class T>
        uint32 Write(const T& value)
        {
            if (WritePos + sizeof(T) > GetCapacity())
            {
                return 0;
            }
            new (Storage.GetData() + WritePos) T(value);
            WritePos += sizeof(T);
            Size = std::max(Size, WritePos);
            return sizeof(T);
        }

        template <class T, class ...TArgs>
        uint32 Emplace(TArgs&& ...args)
        {
            if (WritePos + sizeof(T) > GetCapacity())
            {
                return 0;
            }
            new (Storage.GetData() + WritePos) T(std::move(args)...);
            WritePos += sizeof(T);
            Size = std::max(Size, WritePos);
            return sizeof(T);
        }

        uint32 Read(void* dest, uint32 len)
        {
            len = std::max(GetCapacity() - ReadPos, len);
            memcpy(dest, Storage.GetData() + ReadPos, len);
            ReadPos += len;
            return len;
        }

        template <class T>
        uint32 Read(T& outValue)
        {
            if (ReadPos + sizeof(T) > GetCapacity())
            {
                return 0;
            }
            outValue = *reinterpret_cast<T*>(Storage.GetData() + ReadPos);
            ReadPos += sizeof(T);
            return sizeof(T);
        }

        template <class T>
        T* ReadPtr()
        {
            if (ReadPos + sizeof(T) > GetCapacity())
            {
                return nullptr;
            }
            T* ptr = reinterpret_cast<T*>(Storage.GetData() + ReadPos);
            ReadPos += sizeof(T);
            return ptr;
        }

        void Reset()
        {
            Storage.SetZero();
            WritePos = 0;
            ReadPos = 0;
            Size = 0;
        }
    };

    template <uint32 N>
    using TInlineBuffer = TBuffer<InlineStoragePolicy<N>>;

    using TFixedBuffer = TBuffer<FixedStoragePolicy>;

    using THeapBuffer = TBuffer<HeapStoragePolicy>;
}
