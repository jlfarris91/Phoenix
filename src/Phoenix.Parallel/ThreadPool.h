#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

#include "Task.h"
#include "ChaseLevDeque.h"
#include "MPMCQueue.h"
#include "Phoenix/Platform.h"

namespace Phoenix
{
    // Per-worker work-stealing deque depth.
    constexpr int64 kThreadPoolDequeCapacity = 256;

    class ThreadPool
    {
    public:
        // queueCapacity here sizes the *Task slab* — the pre-allocated pool
        // of TaskSlot objects backing every live Submit.
        ThreadPool(std::string id, uint32 numWorkers, uint32 queueCapacity = 1024);
        ~ThreadPool();

        uint32 GetNumWorkers() const;

        void Shutdown();

        std::shared_ptr<TaskHandle> Submit(const Task& task);
        std::shared_ptr<TaskHandle> Submit(TTaskFunc&& work);

        // Reference-counted busy-period: while count > 0 workers skip the sleep
        // phase and keep spinning, eliminating OS wake latency between task batches.
        // Safe to call from multiple threads concurrently (parallel world ticks).
        void BeginBusyPeriod();
        void EndBusyPeriod();

        bool IsEmpty() const;
        bool WaitIdle(std::chrono::milliseconds maxWaitTime = std::chrono::milliseconds(0)) const;

    private:
        void Worker(uint32 workerId);

        struct alignas(PHX_CACHE_LINE_SIZE) TaskSlot
        {
            Task Body;
            std::atomic<std::uint32_t> NextFree{0};
        };

        static constexpr std::uint32_t kInvalidSlot = 0xFFFFFFFFu;

        Task* SlabAllocate();
        void  SlabFree(Task* task);
        std::uint32_t SlotIndex(const Task* task) const;

        std::string Id;
        std::vector<std::thread> Threads;
        uint32 NumWorkers;
        std::uint32_t SlotCount;
        std::unique_ptr<TaskSlot[]> Slots;
        std::unique_ptr<TChaseLevDeque<Task*, kThreadPoolDequeCapacity>[]> WorkerDeques;
        TMPMCQueue<Task*> SubmissionInbox;

        alignas(PHX_CACHE_LINE_SIZE) std::atomic<bool> Done = false;
        alignas(PHX_CACHE_LINE_SIZE) std::atomic<std::int32_t> InFlight{0};
        alignas(PHX_CACHE_LINE_SIZE) std::atomic<std::uint64_t> SlabFreeHead{0};
        alignas(PHX_CACHE_LINE_SIZE) std::atomic<std::uint32_t> ActiveWorkerCount{0};
        alignas(PHX_CACHE_LINE_SIZE) std::atomic<std::int32_t> SleepingWorkerCount{0};
        alignas(PHX_CACHE_LINE_SIZE) std::atomic<std::uint32_t> BusyPeriod{0};
        std::mutex WakeMutex;
        std::condition_variable WakeCV;
        std::atomic<uint64_t> WakeNotifyCount{0};
    };
}
