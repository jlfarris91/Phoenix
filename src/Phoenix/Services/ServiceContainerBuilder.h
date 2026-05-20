#pragma once

#include <tuple>

#include "ServiceContainer.h"

namespace Phoenix
{
    class IService;
    class ServiceContainerBuilder;

    template <class TFactory>
    concept ServiceFactory = requires(TFactory f, const std::shared_ptr<IServiceLocator>& loc)
    {
        { f(loc) } -> std::convertible_to<std::shared_ptr<IService>>;
    };

    class ServiceRegistrar
    {
    public:
        ServiceRegistrar(ServiceContainerBuilder* builder, const std::shared_ptr<ServiceRegistration>& registration);

        const ServiceRegistrar& As(FName typeId) const;

        template <class TInterface>
        const ServiceRegistrar& As() const
        {
            return this->As(StaticTypeName<TInterface>::TypeId);
        }

        const ServiceRegistrar& AsInterfaces() const;
        const ServiceRegistrar& InstancePerScope() const;

    protected:
        ServiceContainerBuilder* Builder;
        std::shared_ptr<ServiceRegistration> Registration;
    };

    template <class T>
    class TypedServiceRegistrar : public ServiceRegistrar
    {
    public:
        TypedServiceRegistrar(ServiceContainerBuilder* builder, const std::shared_ptr<ServiceRegistration>& registration)
            : ServiceRegistrar(builder, registration)
        {}

        template <class TInterface>
        const TypedServiceRegistrar& As() const
        {
            (void)ServiceRegistrar::As(StaticTypeName<TInterface>::TypeId);
            return *this;
        }

        const TypedServiceRegistrar& AsInterfaces() const
        {
            (void)ServiceRegistrar::AsInterfaces();
            return *this;
        }

        const TypedServiceRegistrar& InstancePerScope() const
        {
            (void)ServiceRegistrar::InstancePerScope();
            return *this;
        }

        template <class... TDeps, class... TValues>
        const TypedServiceRegistrar& WithConstructor(TValues&&... values) const
        {
            Registration->FactoryFunc =
                [valuesTuple = std::make_tuple(std::forward<TValues>(values)...)]
                (const std::shared_ptr<IServiceLocator>& loc) -> std::shared_ptr<IService>
                {
                    return std::apply(
                        [&loc](TValues&&... args) -> std::shared_ptr<IService>
                        {
                            return std::make_shared<T>(
                                loc->ResolveService<TDeps>()...,
                                std::forward<decltype(args)>(args)...);
                        },
                        valuesTuple);
                };
            return *this;
        }
    };

    class ServiceContainerBuilder
    {
    public:
        // 1. Pre-built instance — TService deduced from shared_ptr<T>
        template <class TService>
        TypedServiceRegistrar<TService> Register(std::shared_ptr<TService> instance)
        {
            FName typeId = StaticTypeName<TService>::TypeId;
            std::weak_ptr weakInstance = instance;
            auto registration = MakeRegistration(typeId,
                [weakInstance](const std::shared_ptr<IServiceLocator>&) -> std::shared_ptr<IService>
                {
                    return weakInstance.lock();
                });
            Container.Instances.push_back(instance);
            Container.TypeIdToInstance[typeId] = instance;
            return TypedServiceRegistrar<TService>(this, registration);
        }

        // 2. Value-arg construction (including zero-arg default construction)
        template <class TService, class... TValues>
        TypedServiceRegistrar<TService> Register(TValues&&... values)
        {
            auto registration = MakeRegistration(StaticTypeName<TService>::TypeId,
                [valuesTuple = std::make_tuple(std::forward<TValues>(values)...)]
                (const std::shared_ptr<IServiceLocator>&) -> std::shared_ptr<IService>
                {
                    return std::apply(
                        [](TValues&&... args) -> std::shared_ptr<IService>
                        {
                            return std::make_shared<TService>(std::forward<decltype(args)>(args)...);
                        },
                        valuesTuple);
                });
            return TypedServiceRegistrar<TService>(this, registration);
        }

        // 3. User-defined factory
        template <class TService, ServiceFactory TFactory>
        TypedServiceRegistrar<TService> Register(TFactory&& factory)
        {
            auto registration = MakeRegistration(StaticTypeName<TService>::TypeId,
                [f = std::forward<TFactory>(factory)]
                (const std::shared_ptr<IServiceLocator>& loc) -> std::shared_ptr<IService>
                {
                    return f(loc);
                });
            return TypedServiceRegistrar<TService>(this, registration);
        }

        // Low-level escape hatch
        ServiceRegistrar RegisterService(FName typeId, const ServiceFactoryFunc& factory);

        std::shared_ptr<ServiceContainer> Build(std::shared_ptr<ServiceContainer> parent = {});

    private:
        friend class ServiceRegistrar;

        template <class T>
        friend class TypedServiceRegistrar;

        std::shared_ptr<ServiceRegistration> MakeRegistration(FName typeId, const ServiceFactoryFunc& factory);
        void RegisterServiceAs(const std::shared_ptr<ServiceRegistration>& registration, FName typeId);
        void RegisterServiceAsInterfaces(const std::shared_ptr<ServiceRegistration>& registration);

        ServiceContainer Container;
    };
}
