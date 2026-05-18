#include "ModuleFactory.h"

Phoenix::ModuleFactory& Phoenix::ModuleFactory::RegisterModule(
    const Phoenix::TypeDescriptor& moduleType,
    const std::string& moduleName)
{
    // ModuleFactories.emplace_back([=](const std::shared_ptr<SessionModuleManager>& moduleManager)
    // {
    //     return moduleManager->RegisterModule(moduleType, moduleName);
    // });
    return *this;
}

Phoenix::ModuleFactory& Phoenix::ModuleFactory::RegisterModuleFactory(
    const std::string& moduleName)
{
    return *this;
}

const std::vector<Phoenix::ModuleFactoryFunc>& Phoenix::ModuleFactory::GetModuleFactories() const
{
    return ModuleFactories;
}
