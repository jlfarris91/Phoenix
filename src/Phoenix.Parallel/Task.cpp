#include "Task.h"

#include "TaskHandle.h"
#include "Phoenix/SpinBackoff.h"

using namespace Phoenix;

Task::Task() = default;

Task::Task(Task&& other) noexcept
    : WorkFunc(std::move(other.WorkFunc))
{
}

Task::Task(TTaskFunc&& work)
    : WorkFunc(std::move(work))
{
}

void Task::operator()() const
{
    Handle->bIsCompleted.store(false, std::memory_order_release);
    WorkFunc();
    Handle->bIsCompleted.store(true, std::memory_order_release);
    if (Handle->OnCompletedFunc)
    {
        Handle->OnCompletedFunc();
    }
}

bool Task::WaitAll(const std::vector<std::shared_ptr<TaskHandle>>& handles, std::chrono::milliseconds maxWaitTime)
{
    auto startTime = PHX_SYS_CLOCK_NOW();
    SpinBackoff backoff;

    for (;;)
    {
        bool done = true;
        for (const std::shared_ptr<TaskHandle>& handle : handles)
        {
            if (!handle->IsCompleted())
            {
                done = false;
                break;
            }
        }

        if (done)
        {
            return true;
        }

        if (maxWaitTime.count() > 0 && (PHX_SYS_CLOCK_NOW() - startTime) > maxWaitTime)
        {
            return false;
        }

        backoff.Tick();
    }
}

bool Task::WaitAny(const std::vector<std::shared_ptr<TaskHandle>>& handles, std::chrono::milliseconds maxWaitTime)
{
    auto startTime = PHX_SYS_CLOCK_NOW();
    SpinBackoff backoff;

    for (;;)
    {
        for (const std::shared_ptr<TaskHandle>& handle : handles)
        {
            if (handle->IsCompleted())
            {
                return true;
            }
        }

        if (maxWaitTime.count() > 0 && (PHX_SYS_CLOCK_NOW() - startTime) > maxWaitTime)
        {
            return false;
        }

        backoff.Tick();
    }
}