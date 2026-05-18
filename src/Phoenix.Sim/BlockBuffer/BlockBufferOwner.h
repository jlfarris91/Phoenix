#pragma once

#include "PhoenixSim/BlockBuffer/BlockBuffer.h"

namespace Phoenix
{
    template <class T>
    struct PHOENIX_SIM_API BlockBufferOwner
    {
        PHX_FORCEINLINE uint8* GetBlock(const FName& name)
        {
            return ThisAsT()->GetBuffer().GetBlock(name);
        }

        PHX_FORCEINLINE const uint8* GetBlock(const FName& name) const
        {
            return ThisAsT()->GetBuffer().GetBlock(name);
        }

        template <class TBlock>
        TBlock* GetBlock(const FName& name)
        {
            return ThisAsT()->GetBuffer().template GetBlock<TBlock>(name);
        }

        template <class TBlock>
        const TBlock* GetBlock(const FName& name) const
        {
            return ThisAsT()->GetBuffer().template GetBlock<TBlock>(name);
        }

        template <class TBlock>
        TBlock* GetBlock()
        {
            return ThisAsT()->GetBuffer().template GetBlock<TBlock>();
        }

        template <class TBlock>
        const TBlock* GetBlock() const
        {
            return ThisAsT()->GetBuffer().template GetBlock<TBlock>();
        }

        template <class TBlock>
        TBlock& GetBlockRef()
        {
            return ThisAsT()->GetBuffer().template GetBlockRef<TBlock>();
        }

        template <class TBlock>
        const TBlock& GetBlockRef() const
        {
            return ThisAsT()->GetBuffer().template GetBlockRef<TBlock>();
        }

    private:

        friend T;

        PHX_FORCEINLINE constexpr T* ThisAsT()
        {
            return static_cast<T*>(this);
        }

        PHX_FORCEINLINE constexpr const T* ThisAsT() const
        {
            return static_cast<const T*>(this);
        }

        BlockBuffer Buffer;
    };
}
