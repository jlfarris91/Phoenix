#pragma once

#include "Phoenix.Sim/OffsetRef.h"

namespace Phoenix
{
    struct PHOENIX_SIM_API BlockBufferAllocator
    {
        BlockBufferAllocator(void* base, uint32 offset, uint32 capacity);

        void* Allocate(uint32 size);

        template <class T>
        T* Allocate(uint32 num = 1)
        {
            return static_cast<T*>(Allocate(sizeof(T) * num));
        }

        template <class T>
        TOffsetRef<T> AllocateRef(uint32 num = 1)
        {
            T* ptr = Allocate<T>(num);
            return TOffsetRef<T>::Create(Base, ptr);
        }

    private:

        void* Base = nullptr;
        uint32 Offset = 0;
        uint32 Capacity = 0;
        uint32 Size = 0;
    };
}
