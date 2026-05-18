#include "SessionModuleManager.h"

#include "SessionModule.h"

bool Phoenix::SessionModuleManager::CanRegisterModule(const TypeDescriptor& moduleType) const
{
    return moduleType.IsA<ISessionModule>();
}

void Phoenix::SessionModuleManager::OnModuleInitialize(IModule* module)
{
    ModuleManager::OnModuleInitialize(module);
    Phoenix::Cast<ISessionModule>(module)->WeakSessionEditor = WeakSessionEditor;
}

void Phoenix::SessionModuleManager::OnModuleShutdown(IModule* module)
{
    ModuleManager::OnModuleShutdown(module);
    Phoenix::Cast<ISessionModule>(module)->WeakSessionEditor.reset();
}

