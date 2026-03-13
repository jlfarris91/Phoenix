#pragma once

#include "PhoenixSim/Services/ServiceContainer.h"

namespace Phoenix
{
    class ServiceContainerBuilder;

    struct ServiceRegistrar
    {
        ServiceRegistrar(ServiceContainerBuilder* builder, const std::shared_ptr<IService>& service);

        const ServiceRegistrar& As(const FName& typeId) const;

        template <class TService>
        const ServiceRegistrar& As() const
        {
            return As(TService::StaticTypeName);
        }

        const ServiceRegistrar& AsInterfaces() const;

    private:

        ServiceContainerBuilder* Builder;
        std::shared_ptr<IService> Service;
    };

    class PHOENIX_SIM_API ServiceContainerBuilder
    {
    public:

        ServiceRegistrar RegisterService(const std::shared_ptr<IService>& service);

        template <class TService, class ...TArgs>
        ServiceRegistrar RegisterService(TArgs&&... args)
        {
            std::shared_ptr<TService> service = std::make_shared<TService>(std::forward<TArgs>(args)...);
            return RegisterService(service);
        }

        std::shared_ptr<ServiceContainer> Build();

    private:

        friend struct ServiceRegistrar;

        void RegisterServiceAs(const std::shared_ptr<IService>& service, const FName& typeId);
        void RegisterServiceAsInterfaces(const std::shared_ptr<IService>& service);

        ServiceContainer Container;
    };
}