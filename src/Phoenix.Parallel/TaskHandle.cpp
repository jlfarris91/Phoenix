#include "TaskHandle.h"

#include "Phoenix/SpinBackoff.h"
#include "Phoenix/Platform.h"

using namespace Phoenix;

bool TaskHandle::IsCompleted() const
{
    return bIsCompleted.load(std::memory_order_acquire);
}

bool TaskHandle::WaitForCompleted(std::chrono::milliseconds maxWaitTime) const
{
    auto startTime = PHX_SYS_CLOCK_NOW();
    SpinBackoff backoff;
    while (!IsCompleted())
    {
        backoff.Tick();
        if (maxWaitTime.count() > 0 && (PHX_SYS_CLOCK_NOW() - startTime) > maxWaitTime)
        {
            return false;
        }
    }
    return true;
}

void TaskHandle::OnCompleted(std::function<void()>&& fn)
{
    OnCompletedFunc = std::move(fn);
    if (IsCompleted())
    {
        OnCompletedFunc();
    }
}