#pragma once

#include "Phoenix/Profiling.h"
#include "Phoenix.Sim/Worlds.h"
#include "Phoenix.Parallel/Task.h"
#include "Phoenix.Parallel/TaskQueue.h"

namespace Phoenix
{
    struct PHOENIX_SIM_API WorldTaskQueue
    {
        using TWorldTaskFunc = std::function<void(WorldRef)>;
        using TParallelRangeFunc = std::function<void(WorldRef, uint32, uint32)>;

        static void Schedule(WorldRef world, TTaskFunc&& func)
        {
            std::shared_ptr<TaskQueue> taskQueue = TaskQueue::GetTaskQueue((uint32)world.GetId());
            taskQueue->Enqueue(std::move(func));
        }

        static void Schedule(WorldRef world, TWorldTaskFunc&& func)
        {
            auto worldPtr = &world;
            TTaskFunc wrapper = [=] { func(*worldPtr); };
            Schedule(world, std::move(wrapper));
        }

        template <class _Fx, class ...TArgs>
        static void Schedule(WorldRef world, _Fx&& fx, TArgs&&... args)
        {
            using namespace std::placeholders;
            TWorldTaskFunc wrapper = std::bind(std::forward<_Fx>(fx), _1, std::forward<TArgs>(args)...);
            Schedule(world, std::move(wrapper));
        }

        template <class ...TArgs>
        static void Schedule(WorldRef world, const std::function<void(WorldRef, TArgs...)>& func, TArgs&& ...args)
        {
            std::shared_ptr<TaskQueue> taskQueue = TaskQueue::GetTaskQueue((uint32)world.GetId());
            auto worldPtr = &world;
            taskQueue->Enqueue([=]
            {
                func(*worldPtr, std::forward<TArgs>(args)...);
            });
        }

        static void ScheduleParallelRange(WorldRef world, uint32 total, uint32 chunkSize, TParallelRangeFunc&& func)
        {
            auto worldPtr = &world;
            std::shared_ptr<TaskQueue> taskQueue = TaskQueue::GetTaskQueue((uint32)world.GetId());

            // No workers? Just run synchronously.
            if (taskQueue->GetNumWorkers() == 0)
            {
                func(*worldPtr, 0, total);
                return;
            }

            // Use chunkSize directly so we create ceil(total/chunkSize) tasks — many more
            // than the worker count. Work-stealing rebalances naturally when per-item cost
            // is uneven (e.g. spatially-clustered contact pairs). Previously this divided
            // by numWorkers first, producing exactly N tasks for N workers and stalling the
            // group barrier whenever one chunk was significantly more expensive than others.
            const uint32 actualRange = chunkSize;

            // Only one task would be created — no parallelism benefit, skip the
            // executor round-trip and run inline.
            if (total == 0 || actualRange >= total)
            {
                if (total > 0)
                    func(*worldPtr, 0, total);
                return;
            }

            std::vector<TTaskFunc>& taskGroup = taskQueue->BeginGroup();

            uint32 start = 0;
            while (start != total)
            {
                uint32 len = actualRange;
                len = len > (total - start) ? (total - start) : len;
                taskGroup.emplace_back([=] { func(*worldPtr, start, len); });
                start += len;
            }

            taskQueue->EndGroup();
        }

        template <class _Fx, class ...TArgs>
        static void ScheduleParallelRange(WorldRef world, uint32 total, uint32 chunkSize, _Fx&& fx, TArgs&&... args)
        {
            using namespace std::placeholders;
            TParallelRangeFunc wrapper = std::bind(std::forward<_Fx>(fx), _1, _2, _3, std::forward<TArgs>(args)...);
            ScheduleParallelRange(world, total, chunkSize, std::move(wrapper));
        }

        static void Flush(WorldRef world)
        {
            PHX_PROFILE_ZONE_SCOPED;

            std::shared_ptr<TaskQueue> taskQueue = TaskQueue::GetTaskQueue((uint32)world.GetId());

            // Submit any pending jobs and pause the thread until they finish.
            taskQueue->Flush();
        }
    };
}
