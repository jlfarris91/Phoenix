#pragma once

#include <functional>
#include <memory>
#include <string>

namespace Phoenix
{
    class IService;
    class IServiceLocator;

    typedef std::function<std::shared_ptr<IService>(const std::shared_ptr<IServiceLocator>&)> ServiceFactoryFunc;

    struct ServiceRegistration
    {
        // The type id of the service that this factory creates.
        std::string TypeId;

        // The type ids of any interfaces that the service implements, which can also be used to look up the service.
        std::vector<std::string> BaseIds;

        // A function that creates an instance of the service.
        ServiceFactoryFunc FactoryFunc;

        // If specified, creates a new instance of the service for each scope (e.g. editor) with the given id,
        // instead of sharing a single instance across the entire application.
        bool InstancePerScope = false;
    };
}
