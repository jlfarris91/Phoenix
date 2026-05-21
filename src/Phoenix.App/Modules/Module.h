#pragma once

#include "ContextObjectContainer.h"
#include "Object.h"

namespace Phoenix
{
    class ServiceContainerBuilder;

    class ModuleInitContext : public ContextObjectContainer
    {
    };

    class ModuleLoadContext : public ContextObjectContainer
    {
    };

    class IModule : public IObject
    {
        PHX_DECLARE_TYPE_INTERFACE(IModule)

    public:

        virtual void Register(ServiceContainerBuilder& builder) {}
        virtual void Initialize(ModuleInitContext& context) {}
        virtual void Load(ModuleLoadContext& context) {}
        virtual void Shutdown() {}
    };
}
