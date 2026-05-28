#pragma once

#include <mutex>
#include <queue>

#include "Phoenix/Reflection/Registration.h"

namespace Phoenix
{
    class IDispatcher
    {
        PHX_DECLARE_TYPE_INTERFACE(IDispatcher)
    public:
        virtual ~IDispatcher() = default;

        // Returns the owning thread id.
        virtual std::thread::id GetOwningThreadId() const = 0;

        // Returns true if the current thread is the same as the owning thread.
        virtual bool IsOnOwningThread() const = 0;

        // Dispatches a function to be called on the owning thread.
        // If the current thread is already the owning thread the function is executed immediately.
        virtual void Dispatch(std::function<void()>&& func) = 0;
    };

    class Dispatcher : public IDispatcher
    {
        PHX_DECLARE_TYPE_DERIVED(Dispatcher, IDispatcher)
    public:
        Dispatcher();

        // Returns the owning thread id.
        std::thread::id GetOwningThreadId() const override;

        // Returns true if the current thread is the same as the owning thread.
        bool IsOnOwningThread() const override;

        // Dispatches a function to be called on the owning thread.
        // If the current thread is already the owning thread the function is executed immediately.
        void Dispatch(std::function<void()>&& func) override;

    protected:

        // Flushes the current dispatch queue.
        void FlushDispatchQueue();

    private:
        std::thread::id OwningThreadId;
        std::queue<std::function<void()>> DispatchQueue;
        std::recursive_mutex DispatchQueueMutex;
    };
}
