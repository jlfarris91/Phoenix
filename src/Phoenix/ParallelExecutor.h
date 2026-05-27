#pragma once

#include "Platform.h"

namespace Phoenix
{
    // Interface for a parallel task executor — the single abstraction the ECS
    // job scheduler uses to submit and track concurrent work.
    //
    // Mirrors the IProfiler pattern: define the interface in Phoenix, ship one
    // or more concrete backends (Phoenix.Parallel, Phoenix.Parallel.enkiTS) as
    // separate optional libraries, and let the application install whichever
    // backend it needs via SetParallelExecutor.
    struct PHOENIX_SIM_API IParallelExecutor
    {
        virtual ~IParallelExecutor() = default;

        // Number of dedicated worker threads (excludes the calling / main thread).
        virtual uint32 GetNumWorkers() const = 0;

        // Index of the calling thread in the range [0, GetNumWorkers()].
        // Index 0 is always the main / calling thread; worker threads return 1..N.
        // Used by the ECS job scheduler to select a per-thread CommandBuffer.
        virtual uint32 GetCurrentThreadIndex() const = 0;

        // Submit a fire-and-forget task. Thread-safe: may be called from the
        // main thread AND from within running tasks (continuation submission).
        virtual void Submit(std::function<void()> fn) = 0;

        // Optional: keep workers spinning for the duration of a known-busy phase
        // (e.g. a world update step) so task batches within that phase are picked
        // up immediately without OS wake latency. Reference-counted — safe to call
        // from multiple threads simultaneously (e.g. parallel world ticks).
        virtual void BeginBusyPeriod() {}
        virtual void EndBusyPeriod()   {}
    };

    PHOENIX_SIM_API bool HasParallelExecutor();
    PHOENIX_SIM_API IParallelExecutor& GetParallelExecutor();
    PHOENIX_SIM_API void SetParallelExecutor(IParallelExecutor* executor);
}
