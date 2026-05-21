#include "AppModuleManager.h"

#include "AppModule.h"

void Phoenix::AppModuleManager::Initialize(const std::shared_ptr<Application> &application)
{
    IAppService::Initialize(application);

    for (auto&& module : Modules)
    {
        ModuleInitContext context;
        OnModuleInitialize(module.get());
        module->Initialize(context);
    }
}

bool Phoenix::AppModuleManager::CanRegisterModule(const Phoenix::TypeDescriptor& moduleType) const
{
    return moduleType.IsA<IAppModule>();
}

void Phoenix::AppModuleManager::OnModuleInitialize(IModule* module)
{
    ModuleManager::OnModuleInitialize(module);
    Phoenix::Cast<IAppModule>(module)->WeakApp = GetApplication();
}

void Phoenix::AppModuleManager::OnModuleShutdown(IModule* module)
{
    ModuleManager::OnModuleShutdown(module);
    Phoenix::Cast<IAppModule>(module)->WeakApp.reset();
}
