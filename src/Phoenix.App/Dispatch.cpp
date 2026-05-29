#include "Dispatch.h"

Phoenix::Dispatcher::Dispatcher()
    : OwningThreadId(std::this_thread::get_id())
{
}

std::thread::id Phoenix::Dispatcher::GetOwningThreadId() const
{
    return OwningThreadId;
}

bool Phoenix::Dispatcher::IsOnOwningThread() const
{
    return std::this_thread::get_id() == OwningThreadId;
}

void Phoenix::Dispatcher::SetOwningThread()
{
    OwningThreadId = std::this_thread::get_id();
}

void Phoenix::Dispatcher::Dispatch(std::function<void()>&& func)
{
    if (std::this_thread::get_id() == OwningThreadId)
    {
        func();
        return;
    }

    std::scoped_lock lock(DispatchQueueMutex);
    DispatchQueue.emplace(std::move(func));
}

void Phoenix::Dispatcher::FlushDispatchQueue()
{
    if (!IsOnOwningThread())
    {
        throw new std::exception("ExecuteDispatchQueue must be called on the owning thread.");
    }

    std::scoped_lock lock(DispatchQueueMutex);

    size_t count = DispatchQueue.size();
    for (size_t i = 0; i < count; i++)
    {
        const auto& func = DispatchQueue.front();
        func();
        DispatchQueue.pop();
    }
}


