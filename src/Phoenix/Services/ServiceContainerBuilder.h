#pragma once

#include <tuple>

#include "ServiceConstruct.h"
#include "ServiceContainer.h"
#include "ServiceModule.h"

namespace Phoenix
{
    class IService;
    class ServiceContainerBuilder;

    template <class TFactory>
    concept ServiceFactory = requires(TFactory f, IServiceLocator& loc)
    {
        { f(loc) } -> std::convertible_to<std::shared_ptr<IService>>;
    };

    class ServiceRegistrar
    {
    public:
        ServiceRegistrar(ServiceContainerBuilder* builder, const std::shared_ptr<ServiceRegistration>& registration);

        const ServiceRegistrar& AsSelf() const;

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

        // Explicit dep list — use when T has multiple constructors and auto-detection picks the wrong one.
        template <class... TDeps, class... TValues>
        const TypedServiceRegistrar& WithConstructor(TValues&&... values) const
        {
            Registration->FactoryFunc =
                [valuesTuple = std::make_tuple(std::forward<TValues>(values)...)]
                (IServiceLocator& loc) -> std::shared_ptr<IService>
                {
                    return std::apply(
                        [&loc](auto&&... args) -> std::shared_ptr<IService>
                        {
                            return MakeServiceExplicit<T, TDeps...>(loc, std::forward<decltype(args)>(args)...);
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
                [weakInstance](IServiceLocator&) -> std::shared_ptr<IService>
                {
                    return weakInstance.lock();
                });
            Container.Instances.push_back(instance);
            Container.TypeIdToInstance[typeId] = instance;
            return TypedServiceRegistrar<TService>(this, registration);
        }

        // 2. Value-arg construction (including zero-arg default construction).
        //    Automatically resolves T::Dependencies if present, then injects a child scope
        //    if the constructor accepts one. Use WithConstructor<TDeps...>() to override.
        template <class TService, class... TValues>
        TypedServiceRegistrar<TService> Register(TValues&&... values)
        {
            auto factory = [valuesTuple = std::make_tuple(std::forward<TValues>(values)...)]
                (IServiceLocator& loc) -> std::shared_ptr<IService>
                {
                    return std::apply(
                        [&loc](auto&&... args) -> std::shared_ptr<IService>
                        {
                            return MakeService<TService>(loc, std::forward<decltype(args)>(args)...);
                        },
                        valuesTuple);
                };
            auto registration = MakeRegistration(StaticTypeName<TService>::TypeId, factory);
            return TypedServiceRegistrar<TService>(this, registration);
        }

        // 3. User-defined factory
        template <class TService, ServiceFactory TFactory>
        TypedServiceRegistrar<TService> Register(TFactory&& factory)
        {
            auto registration = MakeRegistration(StaticTypeName<TService>::TypeId,
                [f = std::forward<TFactory>(factory)]
                (IServiceLocator& loc) -> std::shared_ptr<IService>
                {
                    return f(loc);
                });
            return TypedServiceRegistrar<TService>(this, registration);
        }

        // Low-level escape hatch
        ServiceRegistrar RegisterService(FName typeId, const ServiceFactoryFunc& factory);

        void RegisterModule(const IServiceModule& module);

        template <class T, class ...TArgs>
        void RegisterModule(TArgs&& ...args)
        {
            T module(std::forward<TArgs>(args)...);
            RegisterModuleInternal(module);
        }

        std::shared_ptr<IServiceLocator> Build(std::shared_ptr<IServiceLocator> parent = {});


    private:
        friend class ServiceRegistrar;

        template <class T>
        friend class TypedServiceRegistrar;

        std::shared_ptr<ServiceRegistration> MakeRegistration(FName typeId, const ServiceFactoryFunc& factory);
        void RegisterServiceAsSelf(const std::shared_ptr<ServiceRegistration>& shared);
        void RegisterServiceAs(const std::shared_ptr<ServiceRegistration>& registration, FName typeId);
        void RegisterServiceAsInterfaces(const std::shared_ptr<ServiceRegistration>& registration);

        void RegisterModuleInternal(const IServiceModule& module);

        ServiceContainer Container;
    };
}
