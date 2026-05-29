#pragma once

#include "ApplicationFlags.h"
#include "Dispatch.h"
#include "Phoenix/Services/ServiceContainer.h"
#include "Phoenix/Services/IService.h"

namespace Phoenix
{
    class IAppService;

    class Application : public IService
                      , public Dispatcher
                      , public ServiceContainerOwner
    {
        PHX_DECLARE_TYPE_DERIVED(Application, IService)
    public:

        Application(std::shared_ptr<IServiceLocator> locator);

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

        std::shared_ptr<Application> GetSharedSelf()
        {
            return std::static_pointer_cast<Application>(shared_from_this());
        }

        virtual void InitializeInternal();
        virtual void ShutdownInternal();

        EAppStateFlags StateFlags = EAppStateFlags::None;

        std::vector<std::shared_ptr<IAppService>> AppServices;
    };
}
