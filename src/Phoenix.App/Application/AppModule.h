#pragma once

#include "AppContextObject.h"
#include "Modules/Module.h"

namespace Phoenix
{
    class Application;

    class IAppModule : public IModule, public AppContextObject
    {
        PHX_DECLARE_TYPE_DERIVED(IAppModule, IModule)
        friend class AppModuleManager;
    };
}
