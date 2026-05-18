#pragma once

#include <memory>
#include <vector>
#include <functional>
#include "Phoenix/Reflection/TypeDescriptor.h"
#include "Phoenix/Reflection/TypeRegistry.h"

namespace Phoenix
{
    class IModule;
    class ModuleManager;

    // A function that creates a SessionModule given a SessionModuleManager
    using ModuleFactoryFunc = std::function<std::shared_ptr<IModule>(const std::shared_ptr<ModuleManager>&)>;

    class ModuleFactory : public std::enable_shared_from_this<ModuleFactory>
    {
    public:

        ModuleFactory& RegisterModule(const TypeDescriptor& moduleType, const std::string& moduleName = {});

        ModuleFactory& RegisterModuleFactory(const std::string& moduleName);

        const std::vector<ModuleFactoryFunc>& GetModuleFactories() const;

        template <class T>
        ModuleFactory& RegisterModule(const std::string& moduleName = {})
        {
            static_assert(std::is_base_of_v<IModule, T>, "T must derive from IModule");
            return RegisterModule(TypeRegistry::Get<T>(), moduleName);
        }

    private:
        std::vector<ModuleFactoryFunc> ModuleFactories;
    };
}
