#pragma once

#include "AppContextObject.h"
#include "Phoenix/Services/IService.h"

namespace Phoenix
{
    class Application;

    class IAppService : public IService, public AppContextObject
    {
        PHX_DECLARE_TYPE_DERIVED(IAppService, IService)
    public:

        virtual void Initialize(const std::shared_ptr<Application>& application);
        virtual void Shutdown();
    };
}
