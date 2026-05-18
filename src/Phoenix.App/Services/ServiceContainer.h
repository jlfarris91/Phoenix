#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "ServiceRegistration.h"
#include "ServiceLocator.h"

namespace Phoenix
{
    class Session;

    class ServiceContainer : public IServiceLocator
                           , public std::enable_shared_from_this<ServiceContainer>
    {
    public:

        using IServiceLocator::ResolveService;
        using IServiceLocator::ResolveServices;
        using IServiceLocator::GetService;
        using IServiceLocator::GetServices;
        using IServiceLocator::GetAllServices;

        ServiceContainer(const std::shared_ptr<ServiceContainer>& parent = {});

        // Get the parent service locator, if one was provided.
        std::shared_ptr<ServiceContainer> GetParent() const;
        
        std::shared_ptr<IService> ResolveService(const std::string& typeId) override;

        // Get a service that was registered with the given type id, searching this service container and parent service locators.
        std::shared_ptr<IService> GetService(const std::string& typeId) const override;

        // Get all services that were registered with a given type id, searching this service container and parent service locators.
        uint32_t ResolveServices(const std::string& typeId, std::vector<std::shared_ptr<IService>>& outServices) override;

        // Get all services that were registered with a given type id, searching this service container and parent service locators.
        uint32_t GetServices(const std::string& typeId, std::vector<std::shared_ptr<IService>>& outServices) const override;

        // Get the list of all services registered to this service container and parent service locators.
        uint32_t GetAllServices(std::vector<std::shared_ptr<IService>>& outServices) const override;

        // Get a map of services registered to this specific service container, keyed by their type id.
        // Note that this does not include services from parent service locators.
        const std::unordered_map<std::string, std::shared_ptr<IService>>& GetServiceMap() const;

        // Get a map of services registered to this specific service container, keyed by their type id.
        // Note that this does not include services from parent service locators.
        const std::unordered_map<std::string, std::vector<std::shared_ptr<IService>>>& GetServiceAsMap() const;

    private:

        friend class ServiceContainerBuilder;

        uint32_t FindRegistrationsForTypeId(
            const std::string& typeId,
            std::vector<std::shared_ptr<const ServiceRegistration>>& outRegistrations) const;

        std::weak_ptr<ServiceContainer> Parent;

        std::vector<std::shared_ptr<ServiceRegistration>> Registrations;
        std::unordered_map<std::string, std::shared_ptr<ServiceRegistration>> TypeIdToRegistrationMap;
        std::unordered_map<std::string, std::vector<std::shared_ptr<ServiceRegistration>>> BaseIdToRegistrationsMap;

        std::vector<std::shared_ptr<IService>> Instances;
        std::unordered_map<std::string, std::shared_ptr<IService>> TypeIdToInstanceMap;
        std::unordered_map<std::string, std::vector<std::shared_ptr<IService>>> BaseIdToInstancesMap;
    };

    class ServiceContainerOwner : public IServiceLocator
    {
    public:

        const std::shared_ptr<ServiceContainer>& GetServiceContainer() const;

        // Begin IServiceLocator implementation
        using IServiceLocator::ResolveService;
        using IServiceLocator::ResolveServices;
        using IServiceLocator::GetService;
        using IServiceLocator::GetServices;
        std::shared_ptr<IService> ResolveService(const std::string& typeId) override;
        uint32_t ResolveServices(const std::string& typeId, std::vector<std::shared_ptr<IService>>& outServices) override;
        std::shared_ptr<IService> GetService(const std::string& typeId) const override;
        uint32_t GetServices(const std::string& typeId, std::vector<std::shared_ptr<IService>>& outServices) const override;
        uint32_t GetAllServices(std::vector<std::shared_ptr<IService>>& outServices) const override;
        // End IServiceLocator implementation

    protected:
        std::shared_ptr<ServiceContainer> Container;
    };
}
