
#include "Phoenix.Parallel/TaskQueue.h"

#include <mutex>
#include <unordered_map>

#include "Phoenix/Profiling.h"
#include "Phoenix/SpinBackoff.h"

using namespace Phoenix;

namespace
{
    std::unordered_map<uint32, std::shared_ptr<TaskQueue>> gTaskQueues;
    std::mutex gTaskQueueMutex;
}

TaskQueue::TaskQueue(uint32 id, IParallelExecutor* executor)
    : Id(id)
    , Executor(executor)
{
    Groups.reserve(8);
}

std::shared_ptr<TaskQueue> TaskQueue::CreateTaskQueue(uint32 id)
{
    IParallelExecutor* executor = HasParallelExecutor() ? &GetParallelExecutor() : nullptr;
    std::scoped_lock lock(gTaskQueueMutex);
    auto taskQueue = std::make_shared<TaskQueue>(id, executor);
    gTaskQueues[id] = taskQueue;
    return taskQueue;
}

std::shared_ptr<TaskQueue> TaskQueue::GetTaskQueue(uint32 id)
{
    PHX_PROFILE_ZONE_SCOPED;
    std::scoped_lock lock(gTaskQueueMutex);
    auto iter = gTaskQueues.find(id);
    return iter != gTaskQueues.end() ? iter->second : nullptr;
}

bool TaskQueue::ReleaseTaskQueue(uint32 id)
{
    std::scoped_lock lock(gTaskQueueMutex);
    auto iter = gTaskQueues.find(id);
    if (iter == gTaskQueues.end())
        return false;
    gTaskQueues.erase(iter);
    return true;
}

uint32 TaskQueue::GetId() const
{
    return Id;
}

uint32 TaskQueue::GetNumWorkers() const
{
    return Executor ? Executor->GetNumWorkers() : 0;
}

void TaskQueue::Enqueue(TTaskFunc&& work)
{
    if (Groups.empty())
        Groups.emplace_back();
    Groups.back().push_back(std::move(work));
}

std::vector<TTaskFunc>& TaskQueue::BeginGroup(uint32 size)
{
    Groups.emplace_back().reserve(size);
    return Groups.back();
}

void TaskQueue::EndGroup()
{
    Groups.emplace_back();
}

void TaskQueue::Flush()
{
    PHX_PROFILE_ZONE_SCOPED;

    if (!Executor || Executor->GetNumWorkers() == 0)
    {
        for (auto& group : Groups)
            for (auto& fn : group)
                fn();
        Groups.clear();
        return;
    }

    for (auto& group : Groups)
    {
        const uint32 count = (uint32)group.size();
        if (count == 0)
            continue;

        // Each task decrements the counter when it completes. The main thread
        // spins on this counter as the group barrier before starting the next group.
        std::atomic<uint32> remaining{ count };

        for (uint32 i = 0; i < count; ++i)
        {
            TTaskFunc* fnPtr = group.data() + i;
            Executor->Submit([fnPtr, &remaining]()
            {
                (*fnPtr)();
                remaining.fetch_sub(1, std::memory_order_acq_rel);
            });
        }

        SpinBackoff backoff(200'000); // ~1 ms of PAUSE before yielding
        while (remaining.load(std::memory_order_acquire) != 0)
            backoff.Tick();
    }

    Groups.clear();
}
