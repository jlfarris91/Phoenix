#pragma once

#include <unordered_map>
#include <vector>
#include "Module.h"
#include "Phoenix/Reflection/TypeDescriptor.h"
#include "Delegates.h"

namespace Phoenix
{
    class ModuleManager
    {
    public:
        virtual ~ModuleManager() = default;

        IModule* RegisterModule(const std::shared_ptr<IModule>& module);

        bool UnregisterModule(FName moduleTypeId);

        size_t UnregisterAllModules();

        IModule* FindModule(FName moduleTypeId) const;

        template <class T>
        T* FindModule() const
        {
            return static_cast<T*>(FindModule(StaticTypeName<T>::TypeId));
        }

        bool HasModule(FName moduleTypeId) const;

        template <class T>
        bool HasModule() const
        {
            return HasModule(StaticTypeName<T>::TypeId);
        }

        std::vector<IModule*> GetModules() const;

        PHXED_DECLARE_MULTICAST_DELEGATE(ModuleEvent, IModule*);
        ModuleEvent ModuleRegistered;
        ModuleEvent ModuleUnregistered;

    protected:

        virtual bool CanRegisterModule(const TypeDescriptor& moduleType) const;
        virtual void OnModuleRegistered(IModule* module);
        virtual void OnModuleUnregistered(IModule* module);
        virtual void OnModuleInitialize(IModule* module);
        virtual void OnModuleShutdown(IModule* module);

    private:

        IModule* RegisterModuleInternal(const std::shared_ptr<IModule>& module);
        bool UnregisterModuleInternal(const std::shared_ptr<IModule>& module);

        std::vector<std::shared_ptr<IModule>> Modules;
        std::unordered_map<FName, std::shared_ptr<IModule>> ModulesByType;
    };
}
