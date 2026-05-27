
#pragma once

#include "Phoenix/ParallelExecutor.h"
#include "Phoenix.Parallel/ThreadPool.h"

namespace Phoenix
{
    // IParallelExecutor backend backed by the built-in ThreadPool.
    // This is a thin adapter — the ThreadPool already sets gCurrentThreadIndex
    // for its workers in Worker(), so GetCurrentThreadIndex() just reads it.
    class TaskPoolExecutor final : public IParallelExecutor
    {
    public:
        explicit TaskPoolExecutor(ThreadPool& pool)
            : Pool(pool)
        {}

        uint32 GetNumWorkers() const override
        {
            return Pool.GetNumWorkers();
        }

        uint32 GetCurrentThreadIndex() const override
        {
            return Phoenix::GetCurrentThreadIndex();
        }

        void Submit(std::function<void()> fn) override
        {
            Pool.Submit(TTaskFunc(std::move(fn)));
        }

        void BeginBusyPeriod() override { Pool.BeginBusyPeriod(); }
        void EndBusyPeriod()   override { Pool.EndBusyPeriod();   }

    private:
        ThreadPool& Pool;
    };
}
