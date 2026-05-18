
#pragma once

#include <atomic>
#include <memory>

#include "Phoenix/Platform.h"

namespace Phoenix
{
    // Bounded MPMC ring buffer (Vyukov). The Enqueue/Dequeue cursors are written
    // by every producer / every consumer respectively; placing them on the same
    // cache line as each other (or as the read-only Capacity/Mask/Buffer fields)
    // causes the line to ping-pong between cores on every operation. We align
    // each independently-written atomic to its own cache line.
    template <class T>
    class TMPMCQueue
    {
    public:

        TMPMCQueue(size_t capacity)
            : Capacity(capacity)
            , Mask(capacity - 1)
            , Buffer(new Cell[capacity])
            , EnqueuePos(0)
            , DequeuePos(0)
        {
            for (size_t i = 0; i < Capacity; ++i)
            {
                Buffer[i].sequence.store(i, std::memory_order_relaxed);
            }
        }

        size_t GetCapacity() const { return Capacity; }

        bool TryEnqueue(const T& item)
        {
            size_t pos = EnqueuePos.load(std::memory_order_relaxed);
            while (true)
            {
                Cell& cell = Buffer[pos & Mask];
                size_t seq = cell.sequence.load(std::memory_order_relaxed);
                intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos);
                if (diff == 0)
                {
                    if (EnqueuePos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed))
                    {
                        cell.Data = std::move(item);
                        cell.sequence.store(pos + 1, std::memory_order_release);
                        return true;
                    }
                }
                else if (diff < 0)
                {
                    return false;
                }
                else
                {
                    pos = EnqueuePos.load(std::memory_order_relaxed);
                }
            }
        }

        bool TryDequeue(T& outItem)
        {
            size_t pos = DequeuePos.load(std::memory_order_relaxed);
            while (true)
            {
                Cell& cell = Buffer[pos & Mask];
                size_t seq = cell.sequence.load(std::memory_order_relaxed);
                intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos + 1);
                if (diff == 0)
                {
                    if (DequeuePos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed))
                    {
                        outItem = std::move(cell.Data);
                        cell.sequence.store(pos + Capacity, std::memory_order_release);
                        return true;
                    }
                }
                else if (diff < 0)
                {
                    return false;
                }
                else
                {
                    pos = DequeuePos.load(std::memory_order_relaxed);
                }
            }
        }

        bool IsEmpty() const
        {
            return EnqueuePos.load(std::memory_order_relaxed) == DequeuePos.load(std::memory_order_relaxed);
        }

    private:

        // Each cell is independently written by one producer and one consumer.
        // Aligning to a cache line prevents adjacent cells from sharing a line,
        // which would otherwise serialize producers/consumers operating on
        // neighboring slots.
        struct alignas(PHX_CACHE_LINE_SIZE) Cell
        {
            T Data;
            std::atomic<size_t> sequence;
        };

        // Read-only after construction; safe to share a line.
        size_t Capacity;
        size_t Mask;
        std::unique_ptr<Cell[]> Buffer;

        // Hot atomics on their own cache lines. Producers contend on EnqueuePos,
        // consumers contend on DequeuePos; they must not share a line with each
        // other or with the read-only fields above.
        alignas(PHX_CACHE_LINE_SIZE) std::atomic<size_t> EnqueuePos;
        alignas(PHX_CACHE_LINE_SIZE) std::atomic<size_t> DequeuePos;
    };
}
