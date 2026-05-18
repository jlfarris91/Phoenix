#pragma once

#include "Phoenix/Platform.h"
#include "Phoenix/Reflection/Registration.h"
#include "Phoenix/Name.h"

#include "Phoenix.Sim/BlockBuffer/BlockBufferConfig.h"
#include "Phoenix.Sim/BlockBuffer/BlockBufferDefinition.h"

namespace Phoenix
{
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

        // Resets write-watch tracking, opening a fresh tracking window for the next step.
        // Call at the start of each world step.
        void BeginTracking();

        // Snapshots the dirty pages written since BeginTracking into DirtyPageOffsets.
        // Call at the end of the world step, before SyncTo is invoked.
        void EndTracking();

        const std::vector<uint32>& GetDirtyPageOffsets() const;
        uint32 GetDirtyPageSize() const;

        // Copies dirty pages recorded by EndTracking to view. Falls back to a full copy if
        // EndTracking has not yet been called (e.g. first frame, non-Windows, size mismatch).
        void SyncTo(BlockBuffer& view) const;

        // Applies multiple batches of page offsets (e.g. accumulated stale-patch steps) without
        // building an intermediate set. Duplicate pages across batches are applied twice, which is
        // idempotent and cheaper than hashing. Falls back to full copy on size mismatch.
        void SyncTo(BlockBuffer& view, const std::vector<std::vector<uint32>>& stepPages) const;

    private:

        void CopyTo(BlockBuffer& other) const;

        void AllocateMemory(uint32 size);

        std::vector<Block> Blocks;
        BlockBufferMemoryPtr Data = nullptr;
        uint32 Size = 0;
        uint32 BlockSize = 0;
        uint32 AllocSize = 0;

        // Byte offsets of pages written during the last [BeginTracking, EndTracking] window.
        // DirtyPageSize == 0 means EndTracking has not been called yet, so SyncTo falls
        // back to CopyTo. A non-zero page size with an empty offset list means the step
        // wrote nothing — SyncTo is a no-op.
        std::vector<uint32> DirtyPageOffsets;
        uint32 DirtyPageSize = 0;

        // OS page size, set once during AllocateMemory. Used by the additionalOffsets path
        // of SyncTo when DirtyPageSize == 0 (i.e. called on a view buffer, not the source).
        uint32 SystemPageSize = 0;
    };
}