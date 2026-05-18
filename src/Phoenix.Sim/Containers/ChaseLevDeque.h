#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

#include "PhoenixSim/Platform.h"

namespace Phoenix
{
    // TChaseLevDeque<T, Capacity>
    //
    // Single-owner, multi-thief deque for work-stealing schedulers. The blog
    // entry that motivated this work calls out exactly why this structure
    // beats a global MPMC: the owner uses near-non-atomic relaxed loads and a
    // release-store on the fast path; only thieves (and the rare contested
    // last-element case) pay for a CAS. See:
    //
    //   • Chase, D. & Lev, Y. (2005). Dynamic Circular Work-Stealing Deque. SPAA.
    //   • Lê, N. M. et al. (2013). Correct and Efficient Work-Stealing for
    //     Weak Memory Models. PPoPP.
    //
    // We follow the Lê et al. variant: seq_cst CAS on top + a seq_cst fence
    // in PopOwner before reading top. Correct on ARM / POWER as well as x86.
    //
    // The buffer stores T BY VALUE. That works for value-types when T is
    // trivially copyable and small enough that "owner moves out while thief
    // copies" is not racy on the cache-line level — concretely, when T is a
    // pointer or pointer-sized handle. We static_assert this. Callers that
    // want to store fat objects in here should put them in a per-pool slab
    // (see the upcoming TLockFreeSlab) and store T = slab pointer here.
    template <class T, std::int64_t Capacity = 256>
    class TChaseLevDeque
    {
        static_assert((Capacity & (Capacity - 1)) == 0,
            "TChaseLevDeque: Capacity must be a power of two");
        static_assert(Capacity > 0,
            "TChaseLevDeque: Capacity must be positive");
        static_assert(std::is_trivially_copyable_v<T>,
            "TChaseLevDeque: T must be trivially copyable. Store a pointer to "
            "fat objects instead — the slab allocator pattern.");
        static_assert(sizeof(T) <= 8,
            "TChaseLevDeque: T must fit in 8 bytes (pointer-sized) so the "
            "contested-last-element handoff is race-free on real hardware.");

    public:
        using value_type = T;
        static constexpr std::int64_t kCapacity = Capacity;

        TChaseLevDeque() = default;
        TChaseLevDeque(const TChaseLevDeque&) = delete;
        TChaseLevDeque& operator=(const TChaseLevDeque&) = delete;

        // Owner only. Push to the bottom. Returns false if the deque is full.
        // Fast path: one relaxed load + one acquire load + one buffer store
        // + one release-store on Bottom. No CAS.
        bool Push(T item)
        {
            const std::int64_t b = Bottom.load(std::memory_order_relaxed);
            const std::int64_t t = Top.load(std::memory_order_acquire);
            if (b - t >= Capacity)
            {
                return false; // full
            }
            Buffer[b & kMask] = item;
            // Release: ensures the buffer slot write is visible to any thief
            // that observes the bumped Bottom.
            Bottom.store(b + 1, std::memory_order_release);
            return true;
        }

        // Owner only. Pop from the bottom (LIFO — owner's hot working set
        // stays in cache). Returns false if the deque is empty.
        //
        // Fast path: relaxed dec/store on Bottom, seq_cst fence, relaxed read
        // of Top. The fence + later CAS race only occurs in the
        // last-element-vs-thief case.
        bool PopOwner(T& out)
        {
            const std::int64_t b = Bottom.load(std::memory_order_relaxed) - 1;
            Bottom.store(b, std::memory_order_relaxed);

            // The seq_cst fence enforces global ordering between this thread's
            // Bottom store and a thief's Top load. Without it, ARM / POWER
            // could reorder and both threads conclude they own the last slot.
            std::atomic_thread_fence(std::memory_order_seq_cst);

            std::int64_t t = Top.load(std::memory_order_relaxed);
            if (t <= b)
            {
                // Non-empty: read our candidate from the buffer.
                T candidate = Buffer[b & kMask];
                if (t != b)
                {
                    // Uncontested — more than one element, the thief is
                    // racing on a different slot.
                    out = candidate;
                    return true;
                }
                // Last element: race a thief for it.
                std::int64_t expected = t;
                const bool won = Top.compare_exchange_strong(
                    expected, t + 1,
                    std::memory_order_seq_cst,
                    std::memory_order_relaxed);
                // Whether we won or not, the deque is now empty. Reset
                // Bottom so a subsequent Push starts at the right spot.
                Bottom.store(b + 1, std::memory_order_relaxed);
                if (won)
                {
                    out = candidate;
                    return true;
                }
                // Lost: the thief took it. Our local 'candidate' is
                // discarded — for pointer types, this is a no-op.
                return false;
            }
            else
            {
                // Empty: restore Bottom.
                Bottom.store(b + 1, std::memory_order_relaxed);
                return false;
            }
        }

        // Thief only. Steal from the top (FIFO — picks the oldest, largest
        // unit of work, which is more likely to contain further sub-work and
        // helps the thief stay busy longer).
        //
        // Returns false if the deque appears empty OR if our CAS lost to
        // another thief or to the owner. Callers should treat false as
        // "try a different victim" and rotate.
        bool TrySteal(T& out)
        {
            const std::int64_t t = Top.load(std::memory_order_acquire);
            // Pair with the seq_cst fence in PopOwner.
            std::atomic_thread_fence(std::memory_order_seq_cst);
            const std::int64_t b = Bottom.load(std::memory_order_acquire);
            if (t < b)
            {
                T candidate = Buffer[t & kMask];
                std::int64_t expected = t;
                const bool won = Top.compare_exchange_strong(
                    expected, t + 1,
                    std::memory_order_seq_cst,
                    std::memory_order_relaxed);
                if (won)
                {
                    out = candidate;
                    return true;
                }
            }
            return false;
        }

        // Approximate count; reads two atomics without synchronization. Use
        // for stats / heuristics only — not for correctness decisions.
        std::int64_t SizeApprox() const
        {
            const std::int64_t b = Bottom.load(std::memory_order_relaxed);
            const std::int64_t t = Top.load(std::memory_order_relaxed);
            const std::int64_t diff = b - t;
            return diff < 0 ? 0 : diff;
        }

        bool IsEmptyApprox() const
        {
            return SizeApprox() == 0;
        }

    private:
        static constexpr std::int64_t kMask = Capacity - 1;

        // Top is written by stealing thieves via CAS; cache-line padded to
        // avoid false-sharing with Bottom and the buffer.
        alignas(PHX_CACHE_LINE_SIZE) std::atomic<std::int64_t> Top{0};

        // Bottom is written only by the owner; on its own cache line so
        // thief reads don't bounce the owner's exclusive cache line.
        alignas(PHX_CACHE_LINE_SIZE) std::atomic<std::int64_t> Bottom{0};

        // The ring buffer itself, also on its own cache line. Each slot
        // is written exactly once by the owner per cycle and read by either
        // owner or a single thief — no false-sharing within slots.
        alignas(PHX_CACHE_LINE_SIZE) T Buffer[Capacity]{};
    };
}
