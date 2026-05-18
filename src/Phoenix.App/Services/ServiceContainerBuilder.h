#pragma once

#include "ServiceContainer.h"

namespace Phoenix
{
    class IService;
    class ServiceContainerBuilder;

    struct ServiceRegistrar
    {
        ServiceRegistrar(ServiceContainerBuilder* builder, const std::shared_ptr<ServiceRegistration>& registration);

        const ServiceRegistrar& As(const std::string& typeId) const;

        template <class TService>
        const ServiceRegistrar& As() const
        {
            return this->As(std::string(Phoenix::StaticTypeName<TService>::GetQualifiedName()));
        }

        const ServiceRegistrar& AsInterfaces() const;

        const ServiceRegistrar& InstancePerScope() const;

    private:

        ServiceContainerBuilder* Builder;
        std::shared_ptr<ServiceRegistration> Registration;
    };

    class ServiceContainerBuilder
    {
    public:

        ServiceRegistrar RegisterService(const std::string& typeId, const ServiceFactoryFunc& factory);

        ServiceRegistrar RegisterService(const std::shared_ptr<IService>& service);

        template <class TService, class ...TArgs>
        ServiceRegistrar RegisterService(TArgs&&... args)
        {
            ServiceFactoryFunc factory = [args...](const std::shared_ptr<IServiceLocator>&)
            {
                return std::make_shared<TService>(std::forward<TArgs>(args)...);
            };
            return this->RegisterService(std::string(Phoenix::StaticTypeName<TService>::GetQualifiedName()), factory);
        }

        template <class TService>
        ServiceRegistrar RegisterServiceFactory(const ServiceFactoryFunc& factory)
        {
            return this->RegisterService(std::string(Phoenix::StaticTypeName<TService>::GetQualifiedName()), factory);
        }

        std::shared_ptr<ServiceContainer> Build(const std::shared_ptr<ServiceContainer>& parent = {});

    private:

        friend struct ServiceRegistrar;

        void RegisterServiceAs(const std::shared_ptr<ServiceRegistration>& registration, const std::string& typeId);
        void RegisterServiceAsInterfaces(const std::shared_ptr<ServiceRegistration>& registration);

        ServiceContainer Container;
    };
}
