
#pragma once

#include <algorithm>

#include "FixedMemory.h"
#include "PhoenixSim/Platform.h"

namespace Phoenix
{
    class PHOENIX_SIM_API TFixedBuffer
    {
    public:

        using TStorage = TSelfOffsetStorage<uint8>;
        using TElement = TStorage::TElement;

        enum class ESeekOffset
        {
            Begin,
            End,
            Relative
        };

        TFixedBuffer() = default;

        TFixedBuffer(uint32 offset, uint32 capacity)
            : Storage(offset, capacity)
        {
        }

        TFixedBuffer(TStorage::TElement* ptr, uint32 capacity)
            : Storage(ptr, capacity)
        {
        }

        template <class TAllocator>
        TFixedBuffer(TAllocator& allocator, uint32 capacity)
            : Storage(allocator, capacity)
        {
        }

        PHX_FORCEINLINE uint32 GetCapacity() const
        {
            return Storage.GetCapacity();
        }

        PHX_FORCEINLINE static uint32 GetAllocSizeBytes(uint32 capacity)
        {
            return TStorage::GetAllocSizeBytes(capacity);
        }

        PHX_FORCEINLINE uint32 GetAllocSizeBytes() const
        {
            return Storage.GetAllocSizeBytes();
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

    private:

        TStorage Storage;
        uint32 WritePos = 0;
        uint32 ReadPos = 0;
        uint32 Size = 0;
    };
}
