#pragma once

#include <memory>
#include <vector>

#include "Phoenix/Name.h"
#include "Phoenix/Reflection/TypeName.h"

namespace Phoenix
{
    class IService;

    class IServiceLocator
    {
    public:
        virtual ~IServiceLocator() = default;

        // Resolve: get or lazily create an instance for the given type.
        virtual std::shared_ptr<IService> ResolveService(FName typeId) = 0;

        template <class T>
        std::shared_ptr<T> ResolveService()
        {
            return std::static_pointer_cast<T>(ResolveService(StaticTypeName<T>::TypeId));
        }

        // Get: return a cached instance only; null if not yet resolved.
        virtual std::shared_ptr<IService> GetService(FName typeId) const = 0;

        template <class T>
        std::shared_ptr<T> GetService() const
        {
            return std::static_pointer_cast<T>(GetService(StaticTypeName<T>::TypeId));
        }

        // Bulk resolve: resolve all registrations for a type.
        virtual uint32_t ResolveServices(FName typeId, std::vector<std::shared_ptr<IService>>& out) = 0;

        template <class T>
        uint32_t ResolveServices(std::vector<std::shared_ptr<T>>& out)
        {
            std::vector<std::shared_ptr<IService>> services;
            uint32_t count = ResolveServices(StaticTypeName<T>::TypeId, services);
            for (auto& s : services)
                out.push_back(std::static_pointer_cast<T>(s));
            return count;
        }

        // Bulk get: return all cached instances for a type.
        virtual uint32_t GetServices(FName typeId, std::vector<std::shared_ptr<IService>>& out) const = 0;

        template <class T>
        uint32_t GetServices(std::vector<std::shared_ptr<T>>& out) const
        {
            std::vector<std::shared_ptr<IService>> services;
            uint32_t count = GetServices(StaticTypeName<T>::TypeId, services);
            for (auto& s : services)
                out.push_back(std::static_pointer_cast<T>(s));
            return count;
        }

        // Return all resolved instances across this container and any parent.
        virtual uint32_t GetAllServices(std::vector<std::shared_ptr<IService>>& out) const = 0;
    };
}
