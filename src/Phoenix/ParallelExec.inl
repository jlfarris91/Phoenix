#pragma once

#include "ParallelExecutor.h"
#include "SpinBackoff.h"

namespace Phoenix
{
    // Default granularity floor for ParallelForEach.
    constexpr uint32 kDefaultParallelForEachMinBatch = 64;

    // Execute job(start, len) chunks in parallel across the executor's workers.
    // Blocks until all chunks complete before returning.
    template <class TJob>
    void ParallelRange(IParallelExecutor& executor, uint32 total, uint32 minRange, const TJob& job)
    {
        if (total == 0) return;

        if (executor.GetNumWorkers() == 0)
        {
            job(0u, total);
            return;
        }

        const uint32 desiredRange = total / executor.GetNumWorkers();
        const uint32 actualRange = desiredRange < minRange ? minRange : desiredRange;

        // Count tasks upfront so the atomic starts at the right value.
        uint32 taskCount = 0;
        for (uint32 s = 0; s < total; s += (actualRange < (total - s) ? actualRange : (total - s)))
            ++taskCount;

        std::atomic<uint32> remaining{taskCount};

        uint32 start = 0;
        while (start < total)
        {
            const uint32 len = actualRange < (total - start) ? actualRange : (total - start);
            executor.Submit([&job, start, len, &remaining]()
            {
                job(start, len);
                remaining.fetch_sub(1, std::memory_order_acq_rel);
            });
            start += len;
        }

        SpinBackoff backoff(200'000);
        while (remaining.load(std::memory_order_acquire) != 0)
            backoff.Tick();
    }

    // Execute job(i) for every i in [0, num) in parallel across the executor's workers.
    // Blocks until all invocations complete before returning.
    template <class TJob>
    void ParallelForEach(IParallelExecutor& executor, uint32 num, const TJob& job)
    {
        ParallelRange(executor, num, kDefaultParallelForEachMinBatch,
            [&job](uint32 start, uint32 len)
            {
                const uint32 end = start + len;
                for (uint32 i = start; i < end; ++i)
                    job(i);
            });
    }
}
