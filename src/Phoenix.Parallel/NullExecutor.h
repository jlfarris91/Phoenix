
#pragma once

#include "Phoenix/ParallelExecutor.h"

namespace Phoenix
{
    // IParallelExecutor that forces serial execution.
    //
    // GetNumWorkers() returns 0, which causes JobScheduler::Execute to route
    // through ExecuteSerial automatically. Submit is a no-op (tasks submitted
    // through this executor are dropped — callers must not rely on Submit here;
    // use it only as the single-threaded fallback installed at app startup when
    // no real executor is configured).
    class NullExecutor final : public IParallelExecutor
    {
    public:
        uint32 GetNumWorkers() const override { return 0; }
        uint32 GetCurrentThreadIndex() const override { return 0; }
        void Submit(std::function<void()> /*fn*/) override {}
    };
}
