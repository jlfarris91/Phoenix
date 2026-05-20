#include "ServiceContainerBuilder.h"

#include "Phoenix/Reflection/TypeRegistry.h"

#include "IService.h"

using namespace Phoenix;

ServiceRegistrar::ServiceRegistrar(ServiceContainerBuilder* builder, const std::shared_ptr<ServiceRegistration>& registration)
    : Builder(builder)
    , Registration(registration)
{
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
    return ServiceRegistrar(this, MakeRegistration(typeId, factory));
}

std::shared_ptr<ServiceContainer> ServiceContainerBuilder::Build(std::shared_ptr<ServiceContainer> parent)
{
    auto container = std::make_shared<ServiceContainer>(std::move(Container));
    container->Parent = std::move(parent);
    return container;
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
