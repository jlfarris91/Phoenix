#pragma once

#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/InlineCallable.h"
#include "PhoenixSim/Containers/ChaseLevDeque.h"
#include "PhoenixSim/Containers/MPMCQueue.h"

namespace Phoenix
{
    PHOENIX_SIM_API uint32 GetCurrentThreadIndex();
    
    struct PHOENIX_SIM_API TaskHandle
    {
        bool IsCompleted() const;
        bool WaitForCompleted(std::chrono::milliseconds maxWaitTime = std::chrono::milliseconds(0)) const;
        void OnCompleted(std::function<void()>&& fn);

    private:
        friend class Task;
        std::function<void()> OnCompletedFunc;
        std::atomic<bool> bIsCompleted;
    };

    // Task bodies are stored inline — no heap allocation per Submit. The
    // 128-byte capacity comfortably holds typical lambda captures used in the
    // engine (incl. captured std::function<void(WorldRef)> wrappers from
    // WorldTaskQueue, and ECS Submit closures capturing JobBatch by value).
    // If a future call site trips the InlineCallable static_assert, either
    // reduce its capture or bump the capacity here.
    using TTaskFunc = TInlineCallable<void(), 128>;
    
    class PHOENIX_SIM_API Task
    {
    public:

        Task();
        Task(const Task& other) = default;
        Task(Task&& other) noexcept;
        Task(TTaskFunc&& work);

        void operator()() const;

        Task& operator=(const Task& other) = default;
        Task& operator=(Task&& other) = default;

        static PHOENIX_SIM_API bool WaitAll(const std::vector<std::shared_ptr<TaskHandle>>& handles, std::chrono::milliseconds maxWaitTime = std::chrono::milliseconds(0));
        static PHOENIX_SIM_API bool WaitAny(const std::vector<std::shared_ptr<TaskHandle>>& handles, std::chrono::milliseconds maxWaitTime = std::chrono::milliseconds(0));

    private:

        friend class ThreadPool;

        TTaskFunc WorkFunc;
        std::shared_ptr<TaskHandle> Handle;
    };

    class ThreadPool;

    PHOENIX_SIM_API bool HasThreadPool();
    PHOENIX_SIM_API ThreadPool* GetThreadPool();
    PHOENIX_SIM_API void SetThreadPool(const std::string& id, uint32 numWorkers, uint32 queueCapacity = 1024);
    PHOENIX_SIM_API void DestroyThreadPool();

    // Per-worker work-stealing deque depth. 256 slot pointers × NumWorkers
    // gives the engine its parallel-runnable working-set capacity. The slab
    // (below) caps the live Task count across all deques.
    constexpr std::int64_t kThreadPoolDequeCapacity = 256;

    class PHOENIX_SIM_API ThreadPool
    {
    public:
        // queueCapacity here sizes the *Task slab* — the pre-allocated pool
        // of TaskSlot objects backing every live Submit. The name is kept
        // for source compatibility with the old MPMC-based design.
        ThreadPool(std::string id, uint32 numWorkers, uint32 queueCapacity = 1024);
        ~ThreadPool();

        uint32 GetNumWorkers() const;

        void Shutdown();

        std::shared_ptr<TaskHandle> Submit(const Task& task);
        std::shared_ptr<TaskHandle> Submit(TTaskFunc&& work);

        bool IsEmpty() const;
        bool WaitIdle(std::chrono::milliseconds maxWaitTime = std::chrono::milliseconds(0)) const;

    private:

        void Worker(uint32 workerId);

        // ── Slab allocator (pre-allocated TaskSlot pool + lock-free free list)
        //
        // Submit allocates a TaskSlot from this slab and never goes to the heap.
        // The free list is a Treiber stack of slot indices with a 32-bit
        // generation counter packed into the high half of the 64-bit head, so
        // ABA can't happen until ~4 B reuses of the same slot — which would
        // take years at realistic submit rates.
        struct alignas(PHX_CACHE_LINE_SIZE) TaskSlot
        {
            // Default-constructed Task lives here from slab init until pool
            // destruction. Submit assigns into it; the worker resets it after
            // running so captured state (e.g. shared_ptrs in the lambda)
            // doesn't linger in the slot until the next reuse.
            Task Body;
            // Index of the next free slot when this slot is on the free list,
            // or kInvalidSlot when this is the tail. Untouched when in use.
            std::atomic<std::uint32_t> NextFree{0};
        };

        static constexpr std::uint32_t kInvalidSlot = 0xFFFFFFFFu;

        Task* SlabAllocate();
        void  SlabFree(Task* task);
        std::uint32_t SlotIndex(const Task* task) const;

        // Read-only / cold fields.
        std::string Id;
        std::vector<std::thread> Threads;
        uint32 NumWorkers;
        std::uint32_t SlotCount;
        std::unique_ptr<TaskSlot[]> Slots;
        // Chase-Lev deques are single-producer (the owning worker). Tasks
        // submitted from non-worker threads (e.g. the main thread) go
        // through the SubmissionInbox MPMC instead — workers drain it as
        // part of their tryGetWork rotation. This keeps the owner-only
        // invariant of the deque intact while still letting any thread
        // submit work.
        std::unique_ptr<TChaseLevDeque<Task*, kThreadPoolDequeCapacity>[]> WorkerDeques;
        TMPMCQueue<Task*> SubmissionInbox;

        // Hot atomics on their own cache lines.
        //   • Done       — termination flag, read by every worker loop.
        //   • InFlight   — authoritative "work outstanding" count; Submit
        //                  pre-increments, the worker decrements after the
        //                  task body returns. WaitIdle waits on this alone.
        //   • SlabFreeHead — Treiber stack head: high 32 bits ABA generation,
        //                    low 32 bits the slot index (or kInvalidSlot).
        //   • ActiveWorkerCount / SpinningWorkerCount — stats fields.
        alignas(PHX_CACHE_LINE_SIZE) std::atomic<bool> Done = false;
        alignas(PHX_CACHE_LINE_SIZE) std::atomic<std::int32_t> InFlight{0};
        alignas(PHX_CACHE_LINE_SIZE) std::atomic<std::uint64_t> SlabFreeHead{0};
        alignas(PHX_CACHE_LINE_SIZE) std::atomic<std::uint32_t> ActiveWorkerCount{0};
        alignas(PHX_CACHE_LINE_SIZE) std::atomic<std::uint32_t> SpinningWorkerCount{0};
    };

    class PHOENIX_SIM_API TaskQueue
    {
    public:
        TaskQueue(uint32 id, ThreadPool* threadPool = Phoenix::GetThreadPool());
        static std::shared_ptr<TaskQueue> CreateTaskQueue(uint32 id);
        static std::shared_ptr<TaskQueue> GetTaskQueue(uint32 id);
        static bool ReleaseTaskQueue(uint32 id);

        uint32 GetId() const;
        ThreadPool* GetThreadPool() const;
        uint32 GetNumWorkers() const;

        void Enqueue(Task&& task);
        void Enqueue(TTaskFunc&& work);
        void Enqueue(std::vector<Task>&& tasks);

        std::vector<Task>& BeginGroup(uint32 size = 0);
        void EndGroup();

        void Flush();

    private:

        void Complete();

        uint32 Id = 0;
        std::vector<std::vector<Task>> Tasks;
        std::atomic<uint32> CurrTaskIndex;
        std::atomic<bool> bIsCompleted = false;
        ThreadPool* ThreadPool;
    };

    // Default granularity floor for ParallelForEach. Submitting one task per
    // index defeats the queue (push/pop is ~100 ns; a single-index body is
    // typically ~10-100 ns) — see the Cilk-5 work-first principle. Batching
    // by at least this many indices per task keeps the body cost an order of
    // magnitude above the queue cost. Callers with heavier per-index work
    // should use ParallelRange directly and pick their own minRange.
    constexpr uint32 kDefaultParallelForEachMinBatch = 64;

    template <class TThreadPool, class TJob>
    void ParallelRange(TThreadPool& pool, uint32 total, uint32 minRange, const TJob& job)
    {
        uint32 desiredRange = total / pool.GetNumWorkers();
        uint32 actualRange = desiredRange < minRange ? minRange : desiredRange;
        uint32 start = 0;
        while (start != total)
        {
            uint32 len = actualRange;
            len = len > (total - start) ? (total - start) : len;
            pool.Submit([=] { job(start, len); });
            start += len;
        }
        pool.WaitIdle();
    }

    template <class TThreadPool, class TJob>
    void ParallelForEach(TThreadPool& pool, uint32 num, const TJob& job)
    {
        // Forward to ParallelRange so the per-index callable is amortised
        // across batches of at least kDefaultParallelForEachMinBatch items.
        ParallelRange(pool, num, kDefaultParallelForEachMinBatch,
            [&job](uint32 start, uint32 len)
            {
                const uint32 end = start + len;
                for (uint32 i = start; i < end; ++i)
                {
                    job(i);
                }
            });
    }

    template <class TJob>
    void ParallelForEach(uint32 num, const TJob& job)
    {
        if (ThreadPool* threadPool = GetThreadPool())
        {
            ParallelForEach(*threadPool, num, job);
            return;
        }

        // No thread pool? Just run synchronously.
        for (uint32 i = 0; i < num; ++i)
        {
            job(i);
        }
    }

    template <class TJob>
    void ParallelRange(uint32 total, uint32 minRange, const TJob& job)
    {
        if (ThreadPool* threadPool = GetThreadPool())
        {
            ParallelRange(*threadPool, total, minRange, job);
            return;
        }

        // No thread pool? Just run synchronously.
        uint32 desiredRange = total;
        uint32 actualRange = desiredRange < minRange ? minRange : desiredRange;
        uint32 start = 0;
        while (start != total)
        {
            uint32 len = actualRange;
            len = len > (total - start) ? (total - start) : len;
            job(start, len);
            start += len;
        }
    }
}
