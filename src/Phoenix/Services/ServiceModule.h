#pragma once

#include "Phoenix/Reflection/Registration.h"

namespace Phoenix
{
    class ServiceContainerBuilder;

    class IServiceModule
    {
        PHX_DECLARE_TYPE_INTERFACE(IServiceModule)
    public:
        virtual ~IServiceModule() = default;
        virtual void Register(ServiceContainerBuilder& builder) const = 0;
    };
}
