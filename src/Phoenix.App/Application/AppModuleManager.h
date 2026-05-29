#pragma once

#include "AppService.h"
#include "Modules/ModuleManager.h"

namespace Phoenix
{
    class AppModuleManager : public ModuleManager, public IAppService
    {
        PHX_DECLARE_TYPE_DERIVED(AppModuleManager, IAppService)

    public:

        void Initialize(const std::shared_ptr<Application> &application) override;

    protected:

        bool CanRegisterModule(const TypeDescriptor& moduleType) const override;
        void OnModuleInitialize(IModule* module) override;
        void OnModuleShutdown(IModule* module) override;
    };
}
