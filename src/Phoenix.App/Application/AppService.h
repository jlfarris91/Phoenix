#pragma once

#include "AppContextObject.h"
#include "Dispatch.h"
#include "Phoenix/Services/IService.h"

namespace Phoenix
{
    class Application;

    class IAppService : public IService
                      , public IDispatcher
                      , public AppContextObject
    {
        PHX_DECLARE_TYPE_DERIVED(IAppService, IService)
    public:

        virtual void Initialize(const std::shared_ptr<Application>& application);
        virtual void Shutdown();

        virtual void PreTick();
        virtual void Tick();
        virtual void PostTick();

        // Begin IDispatcher implementation
        std::thread::id GetOwningThreadId() const override;
        bool IsOnOwningThread() const override;
        void Dispatch(std::function<void()>&& func) override;
        // End IDispatcher implementation
    };
}
