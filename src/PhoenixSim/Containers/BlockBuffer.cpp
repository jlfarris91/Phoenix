
#include "PhoenixSim/Containers/BlockBuffer.h"

#include <algorithm>
#include <cstring>

#include "FixedArray.h"
#include "PhoenixSim/Logging.h"
#include "PhoenixSim/Profiling.h"

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
        blockSize += block.Definition.Layout.BlockSize;
        allocSize += block.Definition.Layout.AllocSize;
        size = blockSize + allocSize;
    }

    Size = size;
    BlockSize = blockSize;
    AllocSize = allocSize;
    Data = MakeUnique<uint8[]>(Size);

    uint8* dataPtr = Data.get();
    for (Block& block : Blocks)
    {
        uint8* blockPtr = dataPtr + block.Offset;
        uint32 blockTotalSize = block.Definition.Layout.BlockSize + block.Definition.Layout.AllocSize;
        std::memset(blockPtr, 0, blockTotalSize);

        if (block.Definition.ConstructFn.IsBound())
        {
            uint8* blockAllocPtr = blockPtr + block.Definition.Layout.BlockSize;
            BlockBufferAllocator allocator(blockAllocPtr, 0, block.Definition.Layout.AllocSize);
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
    , Size(other.Size)
{
    Data = MakeUnique<uint8[]>(other.Size);
    std::memcpy(Data.get(), other.Data.get(), other.Size);
}

BlockBuffer::BlockBuffer(BlockBuffer&& other) noexcept
    : Blocks(MoveTemp(other.Blocks))
    , Data(MoveTemp(other.Data))
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

    if (Size < other.Size)
    {
        Data.release();
        Data = MakeUnique<uint8[]>(other.Size);
        Size = other.Size;
    }

    std::memcpy(Data.get(), other.Data.get(), other.Size);

    return *this;
}

BlockBuffer& BlockBuffer::operator=(BlockBuffer&& other) noexcept
{
    Data = MoveTemp(other.Data);
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

const TVector<BlockBuffer::Block>& BlockBuffer::GetBlocks() const
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

struct TestBlock : BufferBlockBase
{
    PHX_DECLARE_BLOCK_WITH_ALLOC(TestBlock);

    struct Config
    {
        uint32 MaxItems = 32;
    };

    TFixedArray<int32> Test;
};

TestBlock::TestBlock(BlockBufferAllocator& allocator, const Config& config)
    : Test(allocator, config.MaxItems)
{
}

TestBlock::TestBlock(BlockBufferAllocator& allocator, const Config& config, const TestBlock& other)
    : Test(allocator, config.MaxItems, other.Test)
{
}

BufferBlockLayout TestBlock::Layout(Config config)
{
    BufferBlockLayout layout;
    layout.BlockSize = sizeof(TestBlock);
    layout.AllocSize = TFixedArray<int32>::GetAllocSizeBytes(config.MaxItems);
    return layout;
}

void TestBlock::Construct(void* dest, BlockBufferAllocator& allocator, Config config)
{
    new (dest) TestBlock(allocator, config);
}

void Phoenix::BlockBufferRunTests()
{
    BlockBufferConfig bufferConfig;
    bufferConfig.RegisterBlockWithAlloc<TestBlock>(EBufferBlockType::Dynamic, TestBlock::Config{ 64 });

    BlockBuffer buffer(bufferConfig);

    TestBlock& block = buffer.GetBlockRef<TestBlock>();

    PHX_ASSERT(block.Test.IsEmpty());
    PHX_ASSERT(block.Test.GetCapacity() == 64);

    int32 (*a)[64] = reinterpret_cast<int32(*)[64]>(block.Test.GetData());

    for (uint32 i = 0; i < block.Test.GetCapacity(); ++i)
    {
        block.Test.PushBack(i);
    }

    BlockBuffer buffer2 = buffer;

    TestBlock& block2 = buffer2.GetBlockRef<TestBlock>();

    PHX_ASSERT(block2.Test.IsFull());

    for (uint32 i = 0; i < block2.Test.GetNum(); ++i)
    {
        LogInfo("item[{0}] = {1}", i, block2.Test[i]);
    }
}
