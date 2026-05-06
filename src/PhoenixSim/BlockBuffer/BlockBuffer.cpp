#include "PhoenixSim/BlockBuffer/BlockBuffer.h"

#include <algorithm>
#include <malloc.h>

#include "PhoenixSim/Profiling.h"
#include "PhoenixSim/Utils.h"

using namespace Phoenix;

BlockBufferAllocator::BlockBufferAllocator(void* base, uint32 offset, uint32 capacity)
    : Base(base)
    , Offset(offset)
    , Capacity(capacity)
{
}

void* BlockBufferAllocator::Allocate(uint32 size)
{
    PHX_ASSERT(size + Size <= Capacity);

    if (size + Size > Capacity)
    {
        return nullptr;
    }

    void* ptr = static_cast<uint8*>(Base) + Offset + Size;
    Size += size;
    return ptr;
}

void BlockBufferMemoryDeleter::operator()(void* p) const
{
#ifdef _WIN32
    VirtualFree(p, 0, MEM_RELEASE);
#else
    free(p);
#endif
}

BlockBuffer::Block::Block(const BufferBlockDefinition& definition)
    : Definition(definition)
{
}

BlockBuffer::BlockBuffer(const BlockBufferConfig& config)
{
    Blocks.reserve(config.Definitions.size());
    for (const BufferBlockDefinition& definition : config.Definitions)
    {
        Block& block = Blocks.emplace_back(definition);

        if (definition.LayoutFn.IsBound())
        {
            block.Definition.Layout = definition.LayoutFn.Execute();
        }
    }

    // Sort blocks by priority
    std::ranges::sort(Blocks, [](const Block& a, const Block& b)
    {
        return static_cast<uint8>(a.Definition.SortOrder) < static_cast<uint8>(b.Definition.SortOrder);
    });

    uint32 size = 0;
    uint32 blockSize = 0;
    uint32 allocSize = 0;
    for (Block& block : Blocks)
    {
        block.Offset = size;
        blockSize += block.Definition.Layout.GetHeaderSize();
        allocSize += block.Definition.Layout.GetAllocSize();
        size = blockSize + allocSize;
    }

    AllocateMemory(size);
    BlockSize = blockSize;
    AllocSize = allocSize;

    uint8* dataPtr = Data.get();
    for (Block& block : Blocks)
    {
        uint8* blockPtr = dataPtr + block.Offset;
        uint32 blockTotalSize = block.Definition.Layout.GetAllocSize();
        std::memset(blockPtr, 0, blockTotalSize);

        if (block.Definition.ConstructFn.IsBound())
        {
            uint8* blockAllocPtr = blockPtr + block.Definition.Layout.GetHeaderSize();
            uint32 totalAllocSize = block.Definition.Layout.GetAllocSize();
            BlockBufferAllocator allocator(blockAllocPtr, 0, totalAllocSize);
            block.Definition.ConstructFn.Execute(blockPtr, allocator);
        }
        else if (block.Definition.Type)
        {
            block.Definition.Type->DefaultConstruct(blockPtr);
        }
    }
}

BlockBuffer::BlockBuffer(const BlockBuffer& other)
    : Blocks(other.Blocks)
    , BlockSize(other.BlockSize)
    , AllocSize(other.AllocSize)
{
    AllocateMemory(other.Size);
    std::memcpy(Data.get(), other.Data.get(), other.Size);
}

BlockBuffer::BlockBuffer(BlockBuffer&& other) noexcept
    : Blocks(std::move(other.Blocks))
    , Data(std::move(other.Data))
    , Size(other.Size)
{
    other.Data = nullptr;
    other.Size = 0;
    other.Blocks.clear();
}

BlockBuffer& BlockBuffer::operator=(const BlockBuffer& other)
{
    PHX_PROFILE_ZONE_SCOPED_N("BlockBufferCopy");

    if (&other == this)
        return *this;

    AllocateMemory(other.Size);
    BlockSize = other.BlockSize;
    AllocSize = other.AllocSize;
    Blocks = other.Blocks;

    std::memcpy(Data.get(), other.Data.get(), other.Size);

    return *this;
}

BlockBuffer& BlockBuffer::operator=(BlockBuffer&& other) noexcept
{
    Data = std::move(other.Data);
    BlockSize = other.BlockSize;
    AllocSize = other.AllocSize;
    Size = other.Size;
    return *this;
}

uint8* BlockBuffer::GetData()
{
    return Data.get();
}

const uint8* BlockBuffer::GetData() const
{
    return Data.get();
}

uint32 BlockBuffer::GetSize() const
{
    return Size;
}

const std::vector<BlockBuffer::Block>& BlockBuffer::GetBlocks() const
{
    return Blocks;
}

const BufferBlockDefinition* BlockBuffer::GetBlockDefinition(const FName& name) const
{
    PHX_PROFILE_ZONE_SCOPED;

    for (const Block& block : Blocks)
    {
        if (block.Definition.TypeName == name)
        {
            return &block.Definition;
        }
    }
    return nullptr;
}

uint8* BlockBuffer::GetBlock(const FName& name)
{
    PHX_PROFILE_ZONE_SCOPED;

    for (const Block& block : Blocks)
    {
        if (block.Definition.TypeName == name)
        {
            return Data.get() + block.Offset;
        }
    }
    return nullptr;
}

const uint8* BlockBuffer::GetBlock(const FName& name) const
{
    PHX_PROFILE_ZONE_SCOPED;

    for (const Block& block : Blocks)
    {
        if (block.Definition.TypeName == name)
        {
            return Data.get() + block.Offset;
        }
    }
    return nullptr;
}

void BlockBuffer::CopyTo(BlockBuffer& other) const
{
    PHX_PROFILE_ZONE_SCOPED_N("BlockBufferCopyTo");

    other.AllocateMemory(Size);
    other.Blocks = Blocks;
    other.BlockSize = BlockSize;
    other.AllocSize = AllocSize;

    size_t pageSize = 4096;

#ifdef _WIN32
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    pageSize = sysInfo.dwPageSize;
#endif

    ChunkedParallelCopy(other.Data.get(), Data.get(), Size, pageSize * 256);
}

void BlockBuffer::BeginTracking()
{
#ifdef _WIN32
    if (Data && Size > 0)
    {
        ResetWriteWatch(Data.get(), Size);
    }
#endif
    DirtyPageOffsets.clear();
    DirtyPageSize = 0;
}

void BlockBuffer::EndTracking()
{
#ifdef _WIN32
    if (Data && Size > 0)
    {
        static const DWORD sPageSize = []() -> DWORD
        {
            SYSTEM_INFO info;
            GetSystemInfo(&info);
            return info.dwPageSize;
        }();

        ULONG_PTR pageCount = (Size / sPageSize) + 1;
        thread_local std::vector<void*> sDirtyPages;
        sDirtyPages.resize(pageCount);

        DWORD pageSize;
        // No WRITE_WATCH_FLAG_RESET: BeginTracking owns the reset.
        GetWriteWatch(0, Data.get(), Size, sDirtyPages.data(), &pageCount, &pageSize);

        DirtyPageOffsets.resize(pageCount);
        DirtyPageSize = pageSize;

        const uint8* base = Data.get();
        for (ULONG_PTR i = 0; i < pageCount; ++i)
        {
            DirtyPageOffsets[i] = static_cast<uint32>(
                static_cast<const uint8*>(sDirtyPages[i]) - base);
        }
    }
#endif
}

const std::vector<uint32>& BlockBuffer::GetDirtyPageOffsets() const
{
    return DirtyPageOffsets;
}

uint32 BlockBuffer::GetDirtyPageSize() const
{
    return DirtyPageSize;
}

void BlockBuffer::SyncTo(BlockBuffer& view) const
{
    PHX_PROFILE_ZONE_SCOPED_N("BlockBufferSyncTo");

    if (view.Size != Size)
    {
        CopyTo(view);
        return;
    }

#ifdef _WIN32
    // DirtyPageSize == 0 means EndTracking hasn't been called (e.g. this is a view buffer).
    // Fall back to full copy unless the caller supplied explicit additionalOffsets to apply.
    if (DirtyPageSize == 0)
    {
        CopyTo(view);
        return;
    }

    // View buffers have DirtyPageSize == 0; use SystemPageSize as the fallback.
    const uint32 pageSize = DirtyPageSize != 0 ? DirtyPageSize : SystemPageSize;
    if (pageSize == 0)
    {
        CopyTo(view);
        return;
    }

    const uint8* src = Data.get();
    uint8* dst = view.Data.get();

    for (uint32 offset : DirtyPageOffsets)
    {
        memcpy(dst + offset, src + offset, pageSize);
    }

#else
    memcpy(view.Data.get(), Data.get(), Size);
#endif
}


void BlockBuffer::SyncTo(BlockBuffer& view, const std::vector<std::vector<uint32>>& stepPages) const
{
    PHX_PROFILE_ZONE_SCOPED_N("BlockBufferSyncToSteps");

    if (view.Size != Size)
    {
        CopyTo(view);
        return;
    }

#ifdef _WIN32
    bool hasPages = false;
    for (const auto& step : stepPages)
    {
        if (!step.empty())
        {
            hasPages = true;
            break;
        }
    }

    if (!hasPages)
    {
        CopyTo(view);
        return;
    }

    const uint32 pageSize = SystemPageSize;
    if (pageSize == 0)
    {
        CopyTo(view);
        return;
    }

    const uint8* src = Data.get();
    uint8* dst = view.Data.get();

    for (const auto& step : stepPages)
    {
        for (uint32 offset : step)
        {
            memcpy(dst + offset, src + offset, pageSize);
        }
    }
#else
    memcpy(view.Data.get(), Data.get(), Size);
#endif
}

void BlockBuffer::AllocateMemory(uint32 size)
{
    // The size of the current buffer is already larger.
    if (Size >= size)
    {
        return;
    }

    uint32 pageSize = 4096;

#ifdef _WIN32
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    pageSize = sysInfo.dwPageSize;
#endif

#ifdef _WIN32
    uint8* data = static_cast<uint8*>(VirtualAlloc(
        nullptr, size,
        MEM_RESERVE | MEM_COMMIT | MEM_WRITE_WATCH,
        PAGE_READWRITE));
#else
    void* rawPtr = nullptr;
    posix_memalign(&rawPtr, pageSize, size);
    uint8* data = static_cast<uint8*>(rawPtr);
#endif
    if (!data)
    {
        throw std::bad_alloc();
    }

    Data.reset(data);
    Size = size;
    SystemPageSize = pageSize;
}