#pragma once

#include "InlineCallable.h"

namespace Phoenix
{
    struct TaskHandle;

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
}
