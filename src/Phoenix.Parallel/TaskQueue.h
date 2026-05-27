
#pragma once

#include <memory>
#include <vector>

#include "Phoenix/ParallelExecutor.h"
#include "Task.h"

namespace Phoenix
{
    // Ordered pipeline of parallel task groups (analogous to Unreal's FPipe).
    //
    // Tasks within a group execute concurrently via the supplied IParallelExecutor.
    // Groups are sequential: each group runs to completion before the next begins.
    // This gives a two-level structure:
    //
    //   Enqueue(A), Enqueue(B)          ← group 0: A and B run in parallel
    //   BeginGroup() → push C, D        ← group 1: C and D run in parallel
    //   EndGroup()                      ← sentinel, closes the group
    //   Flush()                         ← group 0 completes, then group 1 completes
    //
    // Per-world queues are stored in a global registry keyed by world ID.
    // FeatureECS creates/destroys them in OnWorldInitialize/OnWorldShutdown.
    class PHOENIX_SIM_API TaskQueue
    {
    public:
        explicit TaskQueue(uint32 id, IParallelExecutor* executor = nullptr);

        static std::shared_ptr<TaskQueue> CreateTaskQueue(uint32 id);
        static std::shared_ptr<TaskQueue> GetTaskQueue(uint32 id);
        static bool ReleaseTaskQueue(uint32 id);

        uint32 GetId() const;
        uint32 GetNumWorkers() const;

        void Enqueue(TTaskFunc&& work);

        std::vector<TTaskFunc>& BeginGroup(uint32 size = 0);
        void EndGroup();

        void Flush();

    private:
        uint32 Id = 0;
        IParallelExecutor* Executor = nullptr;
        std::vector<std::vector<TTaskFunc>> Groups;
    };
}
