#pragma once

#include <mutex>
#include <queue>

#include "ApplicationFlags.h"
#include "Phoenix/Services/ServiceContainer.h"

namespace Phoenix
{
    class IAppService;
    class ServiceContainerBuilder;

    class Application : public std::enable_shared_from_this<Application>
                      , public ServiceContainerOwner
    {
    public:

        struct CtorArgs
        {
            ServiceContainerBuilder* Builder;
        };

        Application(const CtorArgs& args);

        virtual void Initialize();
        virtual void Shutdown();
        virtual void Tick();

        void Run();
        void RequestQuit();
        bool WantsQuit() const;

        bool IsInitializing() const;
        bool IsInitialized() const;
        bool IsShuttingDown() const;
        bool IsShutDown() const;

        // Enqueue a function to be executed on the app thread.
        void Dispatch(std::function<void()>&& function);

    protected:

        virtual void InitializeInternal();
        virtual void ShutdownInternal();

        void ExecuteDispatchQueue();

        EAppStateFlags StateFlags = EAppStateFlags::None;

        std::vector<std::shared_ptr<IAppService>> AppServices;

        std::queue<std::function<void()>> DispatchQueue;
        std::recursive_mutex DispatchQueueMutex;
    };
}
