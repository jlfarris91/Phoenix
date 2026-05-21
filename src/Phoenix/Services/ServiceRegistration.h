#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "Phoenix/Name.h"

namespace Phoenix
{
    class IService;
    class IServiceLocator;

    using ServiceFactoryFunc = std::function<std::shared_ptr<IService>(IServiceLocator&)>;

    struct ServiceRegistration
    {
        FName                TypeId;
        std::vector<FName>   BaseIds;
        ServiceFactoryFunc   FactoryFunc;
        bool                 InstancePerScope = false;
    };
}
