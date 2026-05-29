#include "ServiceContainerBuilder.h"

#include "Phoenix/Reflection/TypeRegistry.h"

#include "IService.h"
#include "ServiceModule.h"

using namespace Phoenix;

ServiceRegistrar::ServiceRegistrar(ServiceContainerBuilder* builder, const std::shared_ptr<ServiceRegistration>& registration)
    : Builder(builder)
    , Registration(registration)
{
}

const ServiceRegistrar& ServiceRegistrar::AsSelf() const
{
    Builder->RegisterServiceAsSelf(Registration);
    return *this;
}

const ServiceRegistrar& ServiceRegistrar::As(FName typeId) const
{
    Builder->RegisterServiceAs(Registration, typeId);
    return *this;
}

const ServiceRegistrar& ServiceRegistrar::AsInterfaces() const
{
    Builder->RegisterServiceAsInterfaces(Registration);
    return *this;
}

const ServiceRegistrar& ServiceRegistrar::InstancePerScope() const
{
    Registration->InstancePerScope = true;
    return *this;
}

std::shared_ptr<ServiceRegistration> ServiceContainerBuilder::MakeRegistration(FName typeId, const ServiceFactoryFunc& factory)
{
    auto registration = std::make_shared<ServiceRegistration>();
    registration->TypeId = typeId;
    registration->FactoryFunc = factory;
    Container.Registrations.push_back(registration);
    Container.TypeIdToRegistration[typeId] = registration;
    return registration;
}

ServiceRegistrar ServiceContainerBuilder::RegisterService(FName typeId, const ServiceFactoryFunc& factory)
{
    return { this, MakeRegistration(typeId, factory) };
}

void ServiceContainerBuilder::RegisterModule(const IServiceModule& module)
{
    RegisterModuleInternal(module);
}

std::shared_ptr<IServiceLocator> ServiceContainerBuilder::Build(std::shared_ptr<IServiceLocator> parent)
{
    auto parentSC = std::dynamic_pointer_cast<ServiceContainer>(std::move(parent));
    auto container = std::make_shared<ServiceContainer>(std::move(Container));
    container->Parent = parentSC;
    return container;
}

void ServiceContainerBuilder::RegisterServiceAsSelf(const std::shared_ptr<ServiceRegistration>& shared)
{
    RegisterServiceAs(shared, shared->TypeId);
}

void ServiceContainerBuilder::RegisterServiceAs(const std::shared_ptr<ServiceRegistration>& registration, FName typeId)
{
    registration->BaseIds.push_back(typeId);
    Container.BaseIdToRegistrations[typeId].push_back(registration);
}

void ServiceContainerBuilder::RegisterServiceAsInterfaces(const std::shared_ptr<ServiceRegistration>& registration)
{
    TypeRegistry::ForEachBaseClass(registration->TypeId, [&](const TypeDescriptor& baseDesc)
    {
        RegisterServiceAs(registration, baseDesc.GetTypeId());
    });
}

void ServiceContainerBuilder::RegisterModuleInternal(const IServiceModule& module)
{
    module.Register(*this);
}
