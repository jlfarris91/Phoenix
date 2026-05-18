#include "Phoenix.Sim/Containers/FixedBlockAllocator.h"

using namespace Phoenix;

void FixedBlockAllocator::Construct(BlockBufferAllocator& allocator, const Config& config)
{
    Configuration = config;
    Blocks.Construct(allocator, config.Capacity);
    BlockData.Construct(allocator, config.Capacity * config.BlockSize);
    IndexMap.Construct(allocator, config.Capacity);
}

BlockBufferLayout FixedBlockAllocator::StaticLayout(const Config& config)
{
    return BlockBufferLayout::For<FixedBlockAllocator>()
        .Container<TFixedArray<Block>>("Blocks", config.Capacity)
        .Container<TFixedStorage<uint8>>("BlockData", config.Capacity * config.BlockSize)
        .Container<TFixedMap<uint32, uint32>>("IndexMap", config.Capacity);
}

uint32 FixedBlockAllocator::GetNumBlocks() const
{
    return Blocks.GetNum();
}

uint32 FixedBlockAllocator::GetNumOccupiedBlocks() const
{
    return NumOccupiedBlocks;
}

uint32 FixedBlockAllocator::GetBlockCapacity() const
{
    return Configuration.Capacity;
}

uint32 FixedBlockAllocator::GetBlockSize() const
{
    return Configuration.BlockSize;
}

bool FixedBlockAllocator::IsEmpty() const
{
    return NumOccupiedBlocks == 0;
}

bool FixedBlockAllocator::IsFull() const
{
    return NumOccupiedBlocks == Configuration.Capacity;
}

bool FixedBlockAllocator::IsValid(Handle handle) const
{
    if (handle.Id == Index<uint32>::None)
    {
        return false;
    }

    uint32 index = GetBlockIndex(handle);
    return Blocks.IsValidIndex(index) && Blocks[index].Id == handle.Id;
}

FixedBlockAllocator::Handle FixedBlockAllocator::Allocate(uint32 userData)
{
    if (IsFull())
    {
        return {};
    }

    uint32 index = AllocateBlock(userData);
    if (index == Index<uint32>::None)
    {
        return {};
    }

    Block& block = Blocks[index];
    return { block.Id };
}

FixedBlockAllocator::Handle FixedBlockAllocator::Allocate(
    uint32 userData,
    const void* source,
    uint32 size)
{
    PHX_ASSERT(size <= Configuration.BlockSize);

    if (IsFull())
    {
        return {};
    }

    uint32 index = AllocateBlock(userData);
    if (index == Index<uint32>::None)
    {
        return {};
    }

    void* data = GetBlockDataPtr(index);
    memcpy(data, source, std::min(size, Configuration.BlockSize));

    Block& block = Blocks[index];
    return { block.Id };
}

bool FixedBlockAllocator::Deallocate(const Handle& handle)
{
    if (!IsValid(handle))
    {
        return false;
    }

    uint32 index = GetBlockIndex(handle);
    Block& block = Blocks[index];
    IndexMap.Remove(block.Id);
    block.Id = Index<uint32>::None;

    --NumOccupiedBlocks;

    return true;
}

void* FixedBlockAllocator::GetPtr(const Handle& handle)
{
    uint32 index = GetBlockIndex(handle);
    if (!Blocks.IsValidIndex(index))
    {
        return nullptr;
    }

    return GetBlockDataPtr(index);
}

const void* FixedBlockAllocator::GetPtr(const Handle& handle) const
{
    uint32 index = GetBlockIndex(handle);
    if (!Blocks.IsValidIndex(index))
    {
        return nullptr;
    }

    return GetBlockDataPtr(index);
}

void FixedBlockAllocator::Compact()
{
    if (Blocks.IsEmpty())
    {
        return;
    }

    //               i ->        <- j
    // Blocks:      [3][-][-][1][-][2]  => [3][2][1][-][-][-]
    // IndexMap:    {1,3}, {2,5}, {3,0} => {1,2}, {2,1}, {3,0}
    uint32 i = 0;
    uint32 j = Blocks.GetNum() - 1;
    while (i < j)
    {
        // When moving forward, skip blocks that are already occupied
        if (IsBlockOccupied(i))
        {
            ++i;
            continue;
        }

        // Walk backwards from the end until we find the first occupied block.
        while (j > i && !IsBlockOccupied(j))
        {
            --j;
        }

        // There are no more blocks to move.
        if (i == j)
        {
            break;
        }

        // Move block at index j to index i
        {
            // Block handle j now points to block at index i
            IndexMap[Blocks[j].Id] = i;

            // Copy the block data to the new address
            Blocks[i] = Blocks[j];
            memcpy(GetBlockDataPtr(i), GetBlockDataPtr(j), Configuration.BlockSize);

            // Invalidate block at index j
            Blocks[j].Id = Index<uint32>::None;
        }

        ++i;
    }

    // Set the size of the array to the number of occupied blocks at the front.
    Blocks.SetSize(NumOccupiedBlocks);
}

FixedBlockAllocator::ConstIter::ConstIter(
    const FixedBlockAllocator* owner,
    uint32 index,
    uint32 numBlocks,
    uint32 blockIdGen)
    : Index(index)
    , NumBlocks(numBlocks)
    , BlockIdGen(blockIdGen)
    , Owner(owner)
{
    Index = std::min(Owner->FindNextOccupiedBlockIndex(Index), NumBlocks);
}

FixedBlockAllocator::Handle FixedBlockAllocator::ConstIter::operator*() const
{
    if (!Owner->Blocks.IsValidIndex(Index))
    {
        return {};
    }

    return { Owner->Blocks[Index].Id };
}

FixedBlockAllocator::ConstIter& FixedBlockAllocator::ConstIter::operator++()
{
    // TODO (jfarris): Uncomment-out this assert once we have refactored AttackAbilitySystem
    // PHX_ASSERT(BlockIdGen == Owner->BlockIdGen);
    Index = std::min(Owner->FindNextOccupiedBlockIndex(Index + 1), NumBlocks);
    return *this;
}

bool FixedBlockAllocator::ConstIter::operator==(const ConstIter& other) const
{
    // Note that we compare the BlockIdGen to detect if the allocator was modified since the iterator was created.
    // If it was, then the iterator is considered invalid and not equal to any other iterator (including itself).
    // This case should be caught by the assert in operator++.
    return BlockIdGen != other.BlockIdGen ||
           NumBlocks != other.NumBlocks ||
           (Owner == other.Owner && Index == other.Index && NumBlocks == other.NumBlocks);
}

bool FixedBlockAllocator::ConstIter::operator!=(const ConstIter& other) const
{
    return !(*this == other);
}

FixedBlockAllocator::ConstIter FixedBlockAllocator::begin() const
{
    return { this, 0, Blocks.GetNum(), BlockIdGen };
}

FixedBlockAllocator::ConstIter FixedBlockAllocator::end() const
{
    return { this, Blocks.GetNum(), Blocks.GetNum(), BlockIdGen };
}

uint32 FixedBlockAllocator::GetBlockIndex(const Handle& handle) const
{
    if (const uint32* indexPtr = IndexMap.GetPtr(handle.Id))
    {
        return *indexPtr;
    }

    return handle.Id;
}

uint8* FixedBlockAllocator::GetBlockDataPtr(uint32 index)
{
    return BlockData.GetData() + static_cast<size_t>(index * Configuration.BlockSize);
}

const uint8* FixedBlockAllocator::GetBlockDataPtr(uint32 index) const
{
    return BlockData.GetData() + static_cast<size_t>(index * Configuration.BlockSize);
}

bool FixedBlockAllocator::IsBlockOccupied(uint32 index) const
{
    return Blocks.IsValidIndex(index) && Blocks[index].Id != Index<uint32>::None;
}

uint32 FixedBlockAllocator::FindNextOccupiedBlockIndex(uint32 index) const
{
    while (index < Blocks.GetNum() && !IsBlockOccupied(index))
    {
        ++index;
    }
    return index;
}

uint32 FixedBlockAllocator::FindFreeBlock() const
{
    for (uint32 i = 0; i < Blocks.GetNum(); ++i)
    {
        if (!IsBlockOccupied(i))
            return i;
    }
    return Index<uint32>::None;
}

uint32 FixedBlockAllocator::AllocateBlock(uint32 userData)
{
    uint32 index = FindFreeBlock();

    // No free blocks, try to add a new one.
    if (index == Index<uint32>::None)
    {
        if (Blocks.IsFull())
        {
            return Index<uint32>::None;
        }

        index = Blocks.GetNum();
        Blocks.PushBack_GetRef();
    }

    Block& block = Blocks[index];
    block.Id = ++BlockIdGen;
    block.UserData = userData;

    uint8* blockDataPtr = GetBlockDataPtr(index);
    memset(blockDataPtr, 0, Configuration.BlockSize);

    IndexMap.Insert(block.Id, index);

    ++NumOccupiedBlocks;

    return index;
}
