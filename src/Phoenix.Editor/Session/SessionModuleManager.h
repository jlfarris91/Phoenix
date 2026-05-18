#pragma once

#include "SessionService.h"
#include "Modules/ModuleManager.h"

namespace Phoenix
{
    class SessionModuleManager : public ModuleManager, public ISessionService
    {
        PHX_DECLARE_TYPE_DERIVED(SessionModuleManager, ISessionService)

    protected:

        virtual bool CanRegisterModule(const TypeDescriptor& moduleType) const override;
        virtual void OnModuleInitialize(IModule* module) override;
        virtual void OnModuleShutdown(IModule* module) override;
    };
}
