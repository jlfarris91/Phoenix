#pragma once

#include "PhoenixSim/Services/ServiceLocator.h"

namespace Phoenix
{
    class Session;

    class PHOENIX_SIM_API ServiceContainer : public ServiceLocator<ServiceContainer>
    {
    public:

        TSharedPtr<IService> GetService(const FName& typeId) const override;

        uint32 GetServices(const FName& typeId, TArray2<TSharedPtr<IService>>& outServices) const override;

        const TArray2<TSharedPtr<IService>>& GetServices() const override;

        const TMap<FName, TSharedPtr<IService>>& GetServiceMap() const;

        const TMap<FName, TArray2<TSharedPtr<IService>>>& GetServiceAsMap() const;

    private:

        friend class ServiceContainerBuilder;

        TArray2<TSharedPtr<IService>> Services;
        TMap<FName, TSharedPtr<IService>> ServiceMap;
        TMap<FName, TArray2<TSharedPtr<IService>>> ServiceAsMap;
    };
}
