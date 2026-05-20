#pragma once

#include "AppService.h"

namespace Phoenix
{
    class IPlatformService : public IAppService
    {
        PHX_DECLARE_TYPE_DERIVED(IPlatformService, IAppService)
    public:
        virtual bool WantsQuit() const = 0;
    };
}
