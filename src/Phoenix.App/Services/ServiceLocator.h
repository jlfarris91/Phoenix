#pragma once

#include <memory>
#include "Phoenix/Reflection/TypeName.h"

namespace Phoenix
{
    class IService;

    class IServiceLocator
    {
    public:
        virtual ~IServiceLocator() = default;

        // Get a service that was registered with the given type id.
        virtual std::shared_ptr<IService> ResolveService(const std::string& typeId) = 0;

        template <class TService>
        std::shared_ptr<TService> ResolveService()
        {
            auto service = this->ResolveService(std::string(Phoenix::StaticTypeName<TService>::GetQualifiedName()));
            return std::static_pointer_cast<TService>(service);
        }

        // Get a service that was registered with the given type id.
        virtual std::shared_ptr<IService> GetService(const std::string& typeId) const = 0;

        template <class TService>
        std::shared_ptr<TService> GetService() const
        {
            auto service = this->GetService(std::string(Phoenix::StaticTypeName<TService>::GetQualifiedName()));
            return std::static_pointer_cast<TService>(service);
        }

        // Get all services that were registered with a given type id.
        // Instantiates instances that have not been instantiated yet.
        virtual uint32_t ResolveServices(const std::string& typeId, std::vector<std::shared_ptr<IService>>& outServices) = 0;

        template <class TService>
        uint32_t ResolveServices(std::vector<std::shared_ptr<TService>>& outServices)
        {
            std::vector<std::shared_ptr<IService>> services;
            this->ResolveServices(std::string(Phoenix::StaticTypeName<TService>::GetQualifiedName()), services);
            for (const std::shared_ptr<IService>& service : services)
            {
                outServices.push_back(std::static_pointer_cast<TService>(service));
            }
            return static_cast<uint32_t>(services.size());
        }

        // Get all services that were registered with a given type id.
        // Instantiates instances that have not been instantiated yet.
        virtual uint32_t GetServices(const std::string& typeId, std::vector<std::shared_ptr<IService>>& outServices) const = 0;

        template <class TService>
        uint32_t GetServices(std::vector<std::shared_ptr<TService>>& outServices) const
        {
            std::vector<std::shared_ptr<IService>> services;
            this->GetServices(std::string(Phoenix::StaticTypeName<TService>::GetQualifiedName()), services);
            for (const std::shared_ptr<IService>& service : services)
            {
                outServices.push_back(std::static_pointer_cast<TService>(service));
            }
            return static_cast<uint32_t>(services.size());
        }

        // Get all services that were registered.
        virtual uint32_t GetAllServices(std::vector<std::shared_ptr<IService>>& outServices) const = 0;
    };
}
