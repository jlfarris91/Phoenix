
#pragma once

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Reflection.h"
#include "PhoenixSim/Name.h"

namespace Phoenix
{
    enum class PHOENIX_SIM_API EBufferBlockType : uint8
    {
        Static,
        Dynamic,
        Scratch
    };

    class PHOENIX_SIM_API BlockBuffer
    {
    public:

        struct BlockDefinition
        {
            FName Name;
            size_t Size = 0;
            uint8 Priority = 0;
            const TypeDescriptor* Type = nullptr;
        };

        struct CtorArgs
        {
            template <class TBlock>
            void RegisterBlock()
            {
                const TypeDescriptor& type = TBlock::GetStaticTypeDescriptor();
                Definitions.emplace_back(type.GetFName(), sizeof(TBlock), (uint8)TBlock::StaticBlockType, &type);
            }

            TArray<BlockDefinition> Definitions;
        };

        struct Block
        {
            Block(const BlockDefinition& definition);

            BlockDefinition Definition;
            size_t Offset = 0;
        };

        BlockBuffer() = default;
        BlockBuffer(const CtorArgs& args);
        BlockBuffer(const BlockBuffer& other);
        BlockBuffer(BlockBuffer&& other) noexcept;

        BlockBuffer& operator=(const BlockBuffer& other);
        BlockBuffer& operator=(BlockBuffer&& other) noexcept;

        uint8* GetData();
        const uint8* GetData() const;

        size_t GetSize() const;

        const TArray<Block>& GetBlocks() const;

        const BlockDefinition* GetBlockDefinition(const FName& name) const;

        uint8* GetBlock(const FName& name);
        const uint8* GetBlock(const FName& name) const;

        template <class TBlock>
        TBlock* GetBlock(const FName& name)
        {
            return static_cast<TBlock>(GetBlock<TBlock>(name));
        }

        template <class TBlock>
        const TBlock* GetBlock(const FName& name) const
        {
            return static_cast<TBlock>(GetBlock<TBlock>(name));
        }

        template <class TBlock>
        TBlock* GetBlock()
        {
            return reinterpret_cast<TBlock*>(GetBlock(TBlock::StaticTypeName));
        }

        template <class TBlock>
        const TBlock* GetBlock() const
        {
            return reinterpret_cast<const TBlock*>(GetBlock(TBlock::StaticTypeName));
        }

        template <class TBlock>
        TBlock& GetBlockRef()
        {
            TBlock* block = GetBlock<TBlock>();
            PHX_ASSERT(block);
            return *block;
        }

        template <class TBlock>
        const TBlock& GetBlockRef() const
        {
            const TBlock* block = GetBlock<TBlock>();
            PHX_ASSERT(block);
            return *block;
        }

    private:

        TArray<Block> Blocks;
        TUniquePtr<uint8[]> Data = nullptr;
        size_t Size = 0;
    };

    struct PHOENIX_SIM_API BufferBlockBase
    {
        virtual ~BufferBlockBase() = default;
        virtual const TypeDescriptor& GetTypeDescriptor() const = 0;
    };

    template <class T>
    struct PHOENIX_SIM_API BlockBufferOwner
    {
        PHX_FORCE_INLINE uint8* GetBlock(const FName& name)
        {
            return ThisAsT()->GetBuffer().GetBlock(name);
        }

        PHX_FORCE_INLINE const uint8* GetBlock(const FName& name) const
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

        PHX_FORCE_INLINE constexpr T* ThisAsT()
        {
            return static_cast<T*>(this);
        }

        PHX_FORCE_INLINE constexpr const T* ThisAsT() const
        {
            return static_cast<const T*>(this);
        }

        BlockBuffer Buffer;
    };
}

#define PHX_DECLARE_BLOCK_BEGIN(block, type) \
    static constexpr Phoenix::EBufferBlockType StaticBlockType = type; \
    PHX_DECLARE_TYPE_BEGIN(block)

#define PHX_DECLARE_BLOCK_END() \
    PHX_DECLARE_TYPE_END()

#define PHX_DECLARE_BLOCK(block, type) \
    static constexpr FName StaticTypeName = #block##_n; \
    static constexpr Phoenix::EBufferBlockType StaticType = type;

#define PHX_DECLARE_BLOCK_STATIC_BEGIN(block) \
    PHX_DECLARE_BLOCK_BEGIN(block, Phoenix::EBufferBlockType::Static)

#define PHX_DECLARE_BLOCK_STATIC(block) \
    PHX_DECLARE_BLOCK_STATIC_BEGIN(block) \
    PHX_DECLARE_BLOCK_END()

#define PHX_DECLARE_BLOCK_DYNAMIC_BEGIN(block) \
    PHX_DECLARE_BLOCK_BEGIN(block, Phoenix::EBufferBlockType::Dynamic)

#define PHX_DECLARE_BLOCK_DYNAMIC(block) \
    PHX_DECLARE_BLOCK_DYNAMIC_BEGIN(block) \
    PHX_DECLARE_BLOCK_END()

#define PHX_DECLARE_BLOCK_SCRATCH_BEGIN(block) \
    PHX_DECLARE_BLOCK_BEGIN(block, Phoenix::EBufferBlockType::Scratch)

#define PHX_DECLARE_BLOCK_SCRATCH(block) \
    PHX_DECLARE_BLOCK_SCRATCH_BEGIN(block) \
    PHX_DECLARE_BLOCK_END()