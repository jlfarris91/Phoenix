#pragma once

#include "PhoenixSim/Services/ServiceContainer.h"

namespace Phoenix
{
    struct ServiceRegistrar
    {
        ServiceRegistrar(ServiceContainerBuilder* builder, const TSharedPtr<IService>& service);

        const ServiceRegistrar& As(const FName& typeId) const;

        template <class TService>
        const ServiceRegistrar& As() const
        {
            return As(TService::StaticTypeName);
        }

    private:

        ServiceContainerBuilder* Builder;
        TSharedPtr<IService> Service;
    };

    class PHOENIX_SIM_API ServiceContainerBuilder
    {
    public:

        ServiceRegistrar RegisterService(const TSharedPtr<IService>& service);

        template <class TService, class ...TArgs>
        ServiceRegistrar RegisterService(TArgs&&... args)
        {
            TSharedPtr<TService> service = MakeShared<TService>(std::forward<TArgs>(args)...);
            return { this, service };
        }

        TSharedPtr<ServiceContainer> Build();

    private:

        friend struct ServiceRegistrar;

        void RegisterServiceAs(const TSharedPtr<IService>& service, const FName& typeId);

        ServiceContainer Container;
    };
}