
#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <thread>

#include "Phoenix/ParallelExecutor.h"
#include "Phoenix/Platform.h"

#include <TaskScheduler.h>

namespace Phoenix
{
    // IParallelExecutor backend powered by enkiTS.
    //
    // Task lifecycle
    // ──────────────
    // Submit() acquires a slot from a fixed-size pool, stores the lambda, and
    // calls AddTaskSetToPipe. The slot's ExecuteRange sets the calling thread's
    // Phoenix thread index (so GetCurrentThreadIndex() works for CommandBuffer
    // selection), runs the lambda, then marks the slot free. The pool is sized
    // at construction time to NumWorkers * kSlotsPerWorker; callers should
    // ensure that number of simultaneously in-flight tasks stays within this
    // bound (typical ECS usage is well within it).
    class EnkiTSExecutor final : public IParallelExecutor
    {
    public:
        static constexpr uint32 kSlotsPerWorker = 16;

        explicit EnkiTSExecutor(uint32 numWorkers = 0);
        ~EnkiTSExecutor() override;

        uint32 GetNumWorkers() const override;
        uint32 GetCurrentThreadIndex() const override;
        void Submit(std::function<void()> fn) override;

        // Register/deregister the calling thread as an external enkiTS thread so
        // it can call Submit(). Must be paired; call Register on entry, Deregister
        // on exit. Set numExternalTaskThreads >= 1 in the config (done by default).
        bool RegisterExternalThread();
        void DeregisterExternalThread();

    private:
        // Each slot is an enkiTS ITaskSet instance with SetSize = 1.
        // alignas(64) keeps slots on separate cache lines so the Free flag
        // doesn't create false sharing between concurrently completing tasks.
        struct alignas(64) LambdaTask : public enki::ITaskSet
        {
            std::function<void()>   Fn;
            std::atomic<bool>       Free{ true };

            LambdaTask() { m_SetSize = 1; }

            void ExecuteRange(enki::TaskSetPartition /*range*/, uint32_t threadNum_) override;
        };

        LambdaTask& AcquireSlot();

        enki::TaskScheduler             Scheduler;
        std::unique_ptr<LambdaTask[]>   Pool;
        uint32                          PoolSize = 0;
        std::thread::id                 MainThreadId;
        std::atomic<uint32>             NextHint{ 0 };
    };
}
