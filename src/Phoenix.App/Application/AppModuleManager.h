#pragma once

#include "AppService.h"
#include "Modules/ModuleManager.h"

namespace Phoenix
{
    class AppModuleManager : public ModuleManager, public IAppService
    {
        PHX_DECLARE_TYPE_DERIVED(AppModuleManager, IAppService)

    protected:

        virtual bool CanRegisterModule(const TypeDescriptor& moduleType) const override;
        virtual void OnModuleInitialize(IModule* module) override;
        virtual void OnModuleShutdown(IModule* module) override;
    };
}
