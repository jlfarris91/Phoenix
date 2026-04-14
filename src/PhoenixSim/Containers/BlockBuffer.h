#pragma once

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Delegates.h"
#include "PhoenixSim/Reflection/Registration.h"
#include "PhoenixSim/Name.h"
#include "PhoenixSim/OffsetRef.h"

namespace Phoenix
{
    enum class PHOENIX_SIM_API EBufferBlockType : uint8
    {
        Static,
        Dynamic,
        Scratch
    };

    enum class PHOENIX_SIM_API EBufferBlockTypeFlags : uint8
    {
        None = 0,
        Static = 1 << (uint8)EBufferBlockType::Static,
        Dynamic = 1 << (uint8)EBufferBlockType::Dynamic,
        Scratch = 1 << (uint8)EBufferBlockType::Scratch,
        All = Static | Dynamic | Scratch
    };

    struct BlockBufferAllocator
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

    struct PHOENIX_SIM_API BufferBlockLayout
    {
        uint32 BlockSize = 0;
        uint32 AllocSize = 0;
    };

    PHX_DECLARE_DELEGATE_RET(FBufferBlockLayout, BufferBlockLayout);
    PHX_DECLARE_DELEGATE(FBufferBlockConstruct, void*, BlockBufferAllocator& allocator);

    // A definition for a block in a block buffer, which includes the block's type name, layout, sort order,
    // and optional factory delegates for layout and construction.
    struct PHOENIX_SIM_API BufferBlockDefinition
    {
        // The type name of the block.
        FName TypeName;

        // The size of the block structure.
        BufferBlockLayout Layout;

        // The sort order of the block.
        uint8 SortOrder = 0;

        // A pointer to the block's type descriptor.
        const TypeDescriptor* Type = nullptr;

        // An optional factory delegate to control construction.
        FBufferBlockLayout LayoutFn;

        // An optional factory delegate to control construction.
        FBufferBlockConstruct ConstructFn;
    };

    // A configuration for a block buffer, which consists of a list of block definitions.
    struct PHOENIX_SIM_API BlockBufferConfig
    {
        BufferBlockDefinition& RegisterBlock(const BufferBlockDefinition& def)
        {
            Definitions.push_back(def);
            return Definitions.back();
        }

        template <class TBlock>
        BufferBlockDefinition& RegisterBlock(EBufferBlockType type, const BufferBlockLayout& layout)
        {
            const TypeDescriptor& typeDescriptor = TypeRegistry::Get<TBlock>();
            return Definitions.emplace_back(
                typeDescriptor.GetTypeId(),
                layout,
                (uint8)type,
                &typeDescriptor);
        }

        template <class TBlock>
        BufferBlockDefinition& RegisterBlock(EBufferBlockType type)
        {
            return RegisterBlock<TBlock>(type, BufferBlockLayout { sizeof(TBlock), 0 });
        }

        template <class TBlock, class ...TVars>
        BufferBlockDefinition& RegisterBlockWithAlloc(EBufferBlockType type, TVars&&... vars)
        {
            BufferBlockDefinition& registration = RegisterBlock<TBlock>(type);
            if constexpr (requires { TBlock::Layout; })
            {
                registration.LayoutFn.BindStatic(&TBlock::Layout, std::forward<TVars>(vars)...);
            }
            if constexpr (requires { TBlock::Construct; })
            {
                registration.ConstructFn.BindStatic(&TBlock::Construct, std::forward<TVars>(vars)...);
            }
            return registration;
        }

        std::vector<BufferBlockDefinition> Definitions;
    };

    // A builder for a block buffer configuration, which provides a fluent interface for registering blocks and building the final layout.
    struct PHOENIX_SIM_API BlockBufferLayoutBuilder
    {
        BufferBlockDefinition& RegisterBlock(const BufferBlockDefinition& definition)
        {
            return Layout.RegisterBlock(definition);
        }

        template <class TBlock>
        BufferBlockDefinition& RegisterBlock(EBufferBlockType type)
        {
            return Layout.RegisterBlock<TBlock>(type);
        }

        template <class TBlock, class ...TVars>
        BufferBlockDefinition& RegisterBlockWithAlloc(EBufferBlockType type, TVars&&... vars)
        {
            return Layout.RegisterBlockWithAlloc<TBlock>(type, std::forward<TVars>(vars)...);
        }

        const BlockBufferConfig& GetLayout() const
        {
            return Layout;
        }

    private:
        BlockBufferConfig Layout;
    };
    
    struct PHOENIX_SIM_API BlockBufferMemoryDeleter
    {
        void operator()(void* p) const;
    };

    using BlockBufferMemoryPtr = std::unique_ptr<uint8, BlockBufferMemoryDeleter>;

    class PHOENIX_SIM_API BlockBuffer
    {
    public:

        struct Block
        {
            Block(const BufferBlockDefinition& definition);

            BufferBlockDefinition Definition;
            uint32 Offset = 0;
        };

        BlockBuffer() = default;
        BlockBuffer(const BlockBufferConfig& config);
        BlockBuffer(const BlockBuffer& other);
        BlockBuffer(BlockBuffer&& other) noexcept;

        BlockBuffer& operator=(const BlockBuffer& other);
        BlockBuffer& operator=(BlockBuffer&& other) noexcept;

        uint8* GetData();
        const uint8* GetData() const;

        uint32 GetSize() const;

        const std::vector<Block>& GetBlocks() const;

        const BufferBlockDefinition* GetBlockDefinition(const FName& name) const;

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
            return reinterpret_cast<TBlock*>(GetBlock(StaticTypeName<TBlock>::TypeId));
        }

        template <class TBlock>
        const TBlock* GetBlock() const
        {
            return reinterpret_cast<const TBlock*>(GetBlock(StaticTypeName<TBlock>::TypeId));
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

        void CopyTo(BlockBuffer& other) const;
        void SyncTo(BlockBuffer& view) const;

    private:

        void AllocateMemory(uint32 size);

        std::vector<Block> Blocks;
        BlockBufferMemoryPtr Data = nullptr;
        uint32 Size = 0;
        uint32 BlockSize = 0;
        uint32 AllocSize = 0;
    };

    struct PHOENIX_SIM_API BufferBlockBase
    {
    };

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

    void BlockBufferRunTests();
}

#define PHX_DECLARE_BLOCK(block) \
    PHX_DECLARE_TYPE(block)

#define PHX_DECLARE_BLOCK_WITH_ALLOC(block) \
    struct Config; \
    block(BlockBufferAllocator& allocator, const Config& config); \
    block(BlockBufferAllocator& allocator, const Config& config, const block& other); \
    static BufferBlockLayout Layout(Config config); \
    static void Construct(void* dest, BlockBufferAllocator& allocator, Config config); \
    PHX_DECLARE_TYPE(block)