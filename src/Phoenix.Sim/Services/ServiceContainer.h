#pragma once

#include "PhoenixSim/Services/ServiceLocator.h"

namespace Phoenix
{
    class Session;

    class PHOENIX_SIM_API ServiceContainer : public ServiceLocator<ServiceContainer>
    {
    public:

        std::shared_ptr<IService> GetService(const FName& typeId) const override;

        uint32 GetServices(const FName& typeId, std::vector<std::shared_ptr<IService>>& outServices) const override;

        const std::vector<std::shared_ptr<IService>>& GetServices() const override;

        const std::unordered_map<FName, std::shared_ptr<IService>>& GetServiceMap() const;

        const std::unordered_map<FName, std::vector<std::shared_ptr<IService>>>& GetServiceAsMap() const;

    private:

        friend class ServiceContainerBuilder;

        std::vector<std::shared_ptr<IService>> Services;
        std::unordered_map<FName, std::shared_ptr<IService>> ServiceMap;
        std::unordered_map<FName, std::vector<std::shared_ptr<IService>>> ServiceAsMap;
    };
}
