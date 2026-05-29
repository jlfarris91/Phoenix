#pragma once

#include "Phoenix/Services/ServiceModule.h"

namespace Phoenix::App::Dev
{
    class SDL3AppServiceModule : public IServiceModule
    {
        PHX_DECLARE_TYPE_DERIVED(SDL3AppServiceModule, IServiceModule)
    public:
        void Register(ServiceContainerBuilder& builder) const override;
    };
}
