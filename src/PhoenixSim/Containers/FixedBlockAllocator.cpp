#include "PhoenixSim/Containers/FixedBlockAllocator.h"

#include <cstring>

Phoenix::uint32 Phoenix::FixedBlockAllocator::GetAllocSizeBytes(const Config& config)
{
    uint32 allocSize = 0;
    allocSize += TFixedArray<Block>::GetAllocSizeBytes(config.Capacity);
    allocSize += TFixedStorage<uint8>::GetAllocSizeBytes(config.Capacity * config.BlockSize);
    allocSize += TFixedMap<uint32, uint32>::GetAllocSizeBytes(config.Capacity);
    return allocSize;
}

Phoenix::uint32 Phoenix::FixedBlockAllocator::GetAllocSizeBytes() const
{
    return GetAllocSizeBytes(Configuration);
}

Phoenix::uint32 Phoenix::FixedBlockAllocator::GetNumBlocks() const
{
    return Blocks.GetNum();
}

Phoenix::uint32 Phoenix::FixedBlockAllocator::GetNumOccupiedBlocks() const
{
    return NumOccupiedBlocks;
}

bool Phoenix::FixedBlockAllocator::IsEmpty() const
{
    return NumOccupiedBlocks == 0;
}

bool Phoenix::FixedBlockAllocator::IsFull() const
{
    return NumOccupiedBlocks == Configuration.Capacity;
}

bool Phoenix::FixedBlockAllocator::IsValid(Handle handle) const
{
    if (handle.Id == Index<uint32>::None)
    {
        return false;
    }

    uint32 index = GetBlockIndex(handle);
    return Blocks.IsValidIndex(index) && Blocks[index].Id == handle.Id;
}

Phoenix::FixedBlockAllocator::Handle Phoenix::FixedBlockAllocator::Allocate(uint32 userData)
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

bool Phoenix::FixedBlockAllocator::Deallocate(const Handle& handle)
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

void* Phoenix::FixedBlockAllocator::GetPtr(const Handle& handle)
{
    uint32 index = GetBlockIndex(handle);
    if (!Blocks.IsValidIndex(index))
    {
        return nullptr;
    }

    return GetBlockDataPtr(index);
}

const void* Phoenix::FixedBlockAllocator::GetPtr(const Handle& handle) const
{
    uint32 index = GetBlockIndex(handle);
    if (!Blocks.IsValidIndex(index))
    {
        return nullptr;
    }

    return GetBlockDataPtr(index);
}

void Phoenix::FixedBlockAllocator::Compact()
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

Phoenix::FixedBlockAllocator::ConstIter::ConstIter(const FixedBlockAllocator* owner, uint32 index): Index(index), Owner(owner)
{
    Index = Owner->FindNextOccupiedBlockIndex(Index);
}

Phoenix::FixedBlockAllocator::Handle Phoenix::FixedBlockAllocator::ConstIter::operator*() const
{
    if (!Owner->Blocks.IsValidIndex(Index))
    {
        return {};
    }

    return { Owner->Blocks[Index].Id };
}

Phoenix::FixedBlockAllocator::ConstIter& Phoenix::FixedBlockAllocator::ConstIter::operator++()
{
    Index = Owner->FindNextOccupiedBlockIndex(Index + 1);
    return *this;
}

Phoenix::FixedBlockAllocator::ConstIter Phoenix::FixedBlockAllocator::begin() const
{
    return { this, FindNextOccupiedBlockIndex(0) };
}

Phoenix::FixedBlockAllocator::ConstIter Phoenix::FixedBlockAllocator::end() const
{
    return { this, Blocks.GetNum() };
}

Phoenix::uint32 Phoenix::FixedBlockAllocator::GetBlockIndex(const Handle& handle) const
{
    if (const uint32* indexPtr = IndexMap.GetPtr(handle.Id))
    {
        return *indexPtr;
    }

    return handle.Id;
}

Phoenix::uint8* Phoenix::FixedBlockAllocator::GetBlockDataPtr(uint32 index)
{
    return BlockData.GetData() + static_cast<size_t>(index * Configuration.BlockSize);
}

const Phoenix::uint8* Phoenix::FixedBlockAllocator::GetBlockDataPtr(uint32 index) const
{
    return BlockData.GetData() + static_cast<size_t>(index * Configuration.BlockSize);
}

bool Phoenix::FixedBlockAllocator::IsBlockOccupied(uint32 index) const
{
    return Blocks.IsValidIndex(index) && Blocks[index].Id != Index<uint32>::None;
}

Phoenix::uint32 Phoenix::FixedBlockAllocator::FindNextOccupiedBlockIndex(uint32 index) const
{
    while (index < Blocks.GetNum() && !IsBlockOccupied(index))
    {
        ++index;
    }
    return index;
}

Phoenix::uint32 Phoenix::FixedBlockAllocator::FindFreeBlock() const
{
    for (uint32 i = 0; i < Blocks.GetNum(); ++i)
    {
        if (!IsBlockOccupied(i))
            return i;
    }
    return Index<uint32>::None;
}

Phoenix::uint32 Phoenix::FixedBlockAllocator::AllocateBlock(uint32 userData)
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
