#include "ModuleManager.h"

#include <algorithm>
#include <ranges>

Phoenix::IModule* Phoenix::ModuleManager::RegisterModule(const std::shared_ptr<IModule>& module)
{
    FName moduleTypeId = module->GetTypeDescriptor().GetTypeId();

    if (!CanRegisterModule(module->GetTypeDescriptor()))
    {
        return nullptr;
    }

    if (ModulesByType.contains(moduleTypeId))
    {
        return ModulesByType.at(moduleTypeId).get();
    }

    return RegisterModuleInternal(module);
}

bool Phoenix::ModuleManager::UnregisterModule(FName moduleTypeId)
{
    auto iter = ModulesByType.find(moduleTypeId);
    return iter != ModulesByType.end() && UnregisterModuleInternal(iter->second);
}

size_t Phoenix::ModuleManager::UnregisterAllModules()
{
    size_t count = 0;
    for (const std::shared_ptr<IModule>& module : Modules | std::views::reverse)
    {
        if (UnregisterModuleInternal(module))
        {
            ++count;
        }
    }
    return count;
}

Phoenix::IModule* Phoenix::ModuleManager::FindModule(FName moduleTypeId) const
{
    auto iter = ModulesByType.find(moduleTypeId);
    return iter != ModulesByType.end() ? iter->second.get() : nullptr;
}

bool Phoenix::ModuleManager::HasModule(FName moduleTypeId) const
{
    return ModulesByType.contains(moduleTypeId);
}

std::vector<std::shared_ptr<Phoenix::IModule>> Phoenix::ModuleManager::GetModules() const
{
    return Modules;
}

bool Phoenix::ModuleManager::CanRegisterModule(const TypeDescriptor& moduleType) const
{
    return moduleType.IsA<IModule>();
}

void Phoenix::ModuleManager::OnModuleRegistered(IModule* module)
{
}

void Phoenix::ModuleManager::OnModuleUnregistered(IModule* module)
{
}

void Phoenix::ModuleManager::OnModuleInitialize(IModule* module)
{
}

void Phoenix::ModuleManager::OnModuleShutdown(IModule* module)
{
}

Phoenix::IModule* Phoenix::ModuleManager::RegisterModuleInternal(const std::shared_ptr<IModule>& module)
{
    FName moduleTypeId = module->GetTypeDescriptor().GetTypeId();
    Modules.push_back(module);
    ModulesByType.insert({ moduleTypeId, module });
    OnModuleRegistered(module.get());
    ModuleRegistered.Broadcast(module.get());
    return module.get();
}

bool Phoenix::ModuleManager::UnregisterModuleInternal(const std::shared_ptr<IModule>& module)
{
    FName moduleTypeId = module->GetTypeDescriptor().GetTypeId();
    if (!ModulesByType.contains(moduleTypeId))
    {
        return false;
    }

    ModuleUnregistered.Broadcast(module.get());
    OnModuleUnregistered(module.get());
    OnModuleShutdown(module.get());
    module->Shutdown();

    ModulesByType.erase(moduleTypeId);
    (void)std::ranges::remove(Modules, module);

    return true;
}
