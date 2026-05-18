#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/InlineCallable.h"
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

    class PHOENIX_SIM_API ThreadPool
    {
    public:
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

        // Read-only / cold fields.
        std::string Id;
        std::vector<std::thread> Threads;
        TMPMCQueue<Task> TaskQueue;
        uint32 NumWorkers;

        // Hot atomics on their own cache lines. Done is read by every worker
        // on every loop iteration; ActiveWorkerCount / SpinningWorkerCount are
        // RMW'd on every task pickup / idle transition. InFlight is the
        // authoritative "work outstanding" counter — incremented in Submit
        // and decremented after task() returns. WaitIdle reads InFlight,
        // closing the TOCTOU race between dequeue and the previous
        // ActiveWorkerCount increment.
        alignas(PHX_CACHE_LINE_SIZE) std::atomic<bool> Done = false;
        alignas(PHX_CACHE_LINE_SIZE) std::atomic<int32_t> InFlight{0};
        alignas(PHX_CACHE_LINE_SIZE) std::atomic<uint32_t> ActiveWorkerCount;
        alignas(PHX_CACHE_LINE_SIZE) std::atomic<uint32_t> SpinningWorkerCount;
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
