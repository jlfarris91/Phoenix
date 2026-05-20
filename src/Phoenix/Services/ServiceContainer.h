#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "IServiceLocator.h"
#include "ServiceRegistration.h"

namespace Phoenix
{
    class ServiceContainer : public IServiceLocator
                           , public std::enable_shared_from_this<ServiceContainer>
    {
    public:
        using IServiceLocator::ResolveService;
        using IServiceLocator::ResolveServices;
        using IServiceLocator::GetService;
        using IServiceLocator::GetServices;

        explicit ServiceContainer(std::shared_ptr<ServiceContainer> parent = {});

        std::shared_ptr<ServiceContainer> GetParent() const;

        std::shared_ptr<IService> ResolveService(FName typeId) override;
        std::shared_ptr<IService> GetService(FName typeId) const override;
        uint32_t ResolveServices(FName typeId, std::vector<std::shared_ptr<IService>>& out) override;
        uint32_t GetServices(FName typeId, std::vector<std::shared_ptr<IService>>& out) const override;
        uint32_t GetAllServices(std::vector<std::shared_ptr<IService>>& out) const override;

        // Returns a snapshot of all resolved instances in this container (no parent walk).
        // Returns by value to prevent iterator invalidation if resolution occurs during iteration.
        std::vector<std::shared_ptr<IService>> GetInstances() const;

        const std::unordered_map<FName, std::shared_ptr<IService>>& GetServiceMap() const;
        const std::unordered_map<FName, std::vector<std::shared_ptr<IService>>>& GetServiceAsMap() const;

    private:
        friend class ServiceContainerBuilder;

        uint32_t FindRegistrations(
            FName typeId,
            std::vector<std::shared_ptr<const ServiceRegistration>>& out) const;

        std::weak_ptr<ServiceContainer> Parent;

        std::vector<std::shared_ptr<ServiceRegistration>>                            Registrations;
        std::unordered_map<FName, std::shared_ptr<ServiceRegistration>>              TypeIdToRegistration;
        std::unordered_map<FName, std::vector<std::shared_ptr<ServiceRegistration>>> BaseIdToRegistrations;

        std::vector<std::shared_ptr<IService>>                            Instances;
        std::unordered_map<FName, std::shared_ptr<IService>>              TypeIdToInstance;
        std::unordered_map<FName, std::vector<std::shared_ptr<IService>>> BaseIdToInstances;
    };

    // Mixin for classes that own a ServiceContainer and want to expose IServiceLocator.
    class ServiceContainerOwner : public IServiceLocator
    {
    public:
        const std::shared_ptr<ServiceContainer>& GetServiceContainer() const;

        using IServiceLocator::ResolveService;
        using IServiceLocator::ResolveServices;
        using IServiceLocator::GetService;
        using IServiceLocator::GetServices;

        std::shared_ptr<IService> ResolveService(FName typeId) override;
        uint32_t ResolveServices(FName typeId, std::vector<std::shared_ptr<IService>>& out) override;
        std::shared_ptr<IService> GetService(FName typeId) const override;
        uint32_t GetServices(FName typeId, std::vector<std::shared_ptr<IService>>& out) const override;
        uint32_t GetAllServices(std::vector<std::shared_ptr<IService>>& out) const override;

    protected:
        std::shared_ptr<ServiceContainer> Container;
    };
}
