#pragma once

#include "ApplicationFlags.h"
#include "Dispatch.h"
#include "Phoenix/Services/ServiceContainer.h"

namespace Phoenix
{
    class IAppService;
    class ServiceContainerBuilder;

    class Application : public std::enable_shared_from_this<Application>
                      , public Dispatcher
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

    protected:

        virtual void InitializeInternal();
        virtual void ShutdownInternal();

        EAppStateFlags StateFlags = EAppStateFlags::None;

        std::vector<std::shared_ptr<IAppService>> AppServices;
    };
}
