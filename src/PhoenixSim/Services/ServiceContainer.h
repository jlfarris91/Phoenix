#pragma once

#include "PhoenixSim/Services/ServiceLocator.h"

namespace Phoenix
{
    class Session;

    class PHOENIX_SIM_API ServiceContainer : public ServiceLocator<ServiceContainer>
    {
    public:

        TSharedPtr<IService> GetService(const FName& typeId) const override;

        uint32 GetServices(const FName& typeId, TVector<TSharedPtr<IService>>& outServices) const override;

        const TVector<TSharedPtr<IService>>& GetServices() const override;

        const std::unordered_map<FName, TSharedPtr<IService>>& GetServiceMap() const;

        const std::unordered_map<FName, TVector<TSharedPtr<IService>>>& GetServiceAsMap() const;

    private:

        friend class ServiceContainerBuilder;

        TVector<TSharedPtr<IService>> Services;
        std::unordered_map<FName, TSharedPtr<IService>> ServiceMap;
        std::unordered_map<FName, TVector<TSharedPtr<IService>>> ServiceAsMap;
    };
}
