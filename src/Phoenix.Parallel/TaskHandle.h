#pragma once

#include <atomic>
#include <chrono>
#include <functional>

namespace Phoenix
{
    struct TaskHandle
    {
        bool IsCompleted() const;
        bool WaitForCompleted(std::chrono::milliseconds maxWaitTime = std::chrono::milliseconds(0)) const;
        void OnCompleted(std::function<void()>&& fn);

    private:
        friend class Task;
        std::function<void()> OnCompletedFunc;
        std::atomic<bool> bIsCompleted;
    };

}
