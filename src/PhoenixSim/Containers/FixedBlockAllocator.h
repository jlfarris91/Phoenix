
#pragma once

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Containers/BlockBuffer.h"
#include "PhoenixSim/Containers/FixedArray.h"
#include "PhoenixSim/Containers/FixedMap.h"

namespace Phoenix
{
    class PHOENIX_SIM_API FixedBlockAllocator
    {
    public:

        struct Config
        {
            uint32 BlockSize = 0;
            uint32 Capacity = 0;
        };

        struct Handle
        {
            uint32 Id = Index<uint32>::None;
        };

        struct Block
        {
            uint32 Id = 0;
            uint32 UserData = 0;
        };

        FixedBlockAllocator() = default;

        template <class TAllocator>
        FixedBlockAllocator(TAllocator& allocator, const Config& config)
            : Configuration(config)
            , Blocks(allocator, config.Capacity)
            , BlockData(allocator, config.Capacity * config.BlockSize)
            , IndexMap(allocator, config.Capacity)
        {
        }

        template <class TAllocator>
        FixedBlockAllocator(TAllocator& allocator, uint32 capacity, const FixedBlockAllocator& other)
            : Configuration({ .BlockSize = other.Configuration.BlockSize, .Capacity = capacity })
            , BlockIdGen(other.BlockIdGen)
            , NumOccupiedBlocks(other.NumOccupiedBlocks)
            , Blocks(allocator, capacity, other.Blocks)
            , BlockData(allocator, capacity * other.Configuration.BlockSize, other.BlockData)
            , IndexMap(allocator, capacity, other.IndexMap)
        {
        }

        static uint32 GetAllocSizeBytes(const Config& config);

        uint32 GetAllocSizeBytes() const;

        uint32 GetNumBlocks() const;

        uint32 GetNumOccupiedBlocks() const;

        bool IsEmpty() const;

        bool IsFull() const;

        bool IsValid(Handle handle) const;

        Handle Allocate(uint32 userData);

        template <class T, class ...TArgs>
        Handle Allocate(uint32 userData, TArgs&& ...args)
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

            void* blockDataPtr = GetBlockDataPtr(index);

            uint32 allocSize = Configuration.BlockSize - sizeof(T);
            BlockBufferAllocator allocator(blockDataPtr, sizeof(T), allocSize);
            new (blockDataPtr) T(allocator, allocSize, std::forward<TArgs>(args)...);

            Block& block = Blocks[index];
            return { block.Id };
        }

        bool Deallocate(const Handle& handle);

        void* GetPtr(const Handle& handle);

        const void* GetPtr(const Handle& handle) const;

        template <class T>
        T* GetPtr(Handle handle)
        {
            return static_cast<T*>(GetPtr(handle));
        }

        template <class T>
        const T* GetPtr(Handle handle) const
        {
            return static_cast<const T*>(GetPtr(handle));
        }

        // Re-organize blocks so that all occupied blocks are at the front.
        void Compact();

        struct ConstIter
        {
            ConstIter(const FixedBlockAllocator* owner, uint32 index, uint32 numBlocks, uint32 blockIdGen);

            Handle operator*() const;

            ConstIter& operator++();

            bool operator==(const ConstIter& other) const;
            bool operator!=(const ConstIter& other) const;

            uint32 Index;
            uint32 NumBlocks;
            uint32 BlockIdGen;
            const FixedBlockAllocator* Owner;
        };

        ConstIter begin() const;
        ConstIter end() const;

    private:

        // Gets the index of a block given a block handle.
        uint32 GetBlockIndex(const Handle& handle) const;

        uint8* GetBlockDataPtr(uint32 index);

        const uint8* GetBlockDataPtr(uint32 index) const;

        // Returns true if the block at the index is occupied.
        bool IsBlockOccupied(uint32 index) const;

        uint32 FindNextOccupiedBlockIndex(uint32 index) const;

        // Returns the index of the first unoccupied block or -1 if there are no free blocks.
        uint32 FindFreeBlock() const;

        // Returns the index of a block or -1 if the array is full.
        uint32 AllocateBlock(uint32 userData);

        Config Configuration;
        uint32 BlockIdGen = 0;
        uint32 NumOccupiedBlocks = 0;
        TFixedArray<Block> Blocks;
        TFixedStorage<uint8> BlockData;
        TFixedMap<uint32, uint32> IndexMap;
    };
}