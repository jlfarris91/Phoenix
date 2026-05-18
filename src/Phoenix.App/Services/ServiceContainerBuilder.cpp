#include "ServiceContainerBuilder.h"

#include "Phoenix/Reflection/TypeRegistry.h"

#include "Service.h"
#include "ServiceContainer.h"

using namespace Phoenix;

ServiceRegistrar::ServiceRegistrar(ServiceContainerBuilder* builder, const std::shared_ptr<ServiceRegistration>& registration)
    : Builder(builder)
    , Registration(registration)
{
}

const ServiceRegistrar& ServiceRegistrar::As(const std::string& typeId) const
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

ServiceRegistrar ServiceContainerBuilder::RegisterService(const std::string& typeId, const ServiceFactoryFunc& factory)
{
    auto registration = std::make_shared<ServiceRegistration>();
    registration->TypeId = typeId;
    registration->FactoryFunc = factory;
    Container.Registrations.push_back(registration);
    Container.TypeIdToRegistrationMap[typeId] = registration;
    return { this, registration };
}

ServiceRegistrar ServiceContainerBuilder::RegisterService(const std::shared_ptr<IService>& service)
{
    const std::string& qualifiedName = service->GetTypeDescriptor().GetQualifiedName();
    std::weak_ptr weakService = service;
    auto factoryFunc = [weakService](const std::shared_ptr<IServiceLocator>&)
    {
        return weakService.lock();
    };
    return RegisterService(qualifiedName, factoryFunc);
}

std::shared_ptr<ServiceContainer> ServiceContainerBuilder::Build(const std::shared_ptr<ServiceContainer>& parent)
{
    auto container = std::make_shared<ServiceContainer>(std::move(Container));
    container->Parent = parent;
    return container;
}

void ServiceContainerBuilder::RegisterServiceAs(const std::shared_ptr<ServiceRegistration>& registration, const std::string& typeId)
{
    registration->BaseIds.push_back(typeId);
    Container.BaseIdToRegistrationsMap[typeId].push_back(registration);
}

void ServiceContainerBuilder::RegisterServiceAsInterfaces(const std::shared_ptr<ServiceRegistration>& registration)
{
    FName typeId(registration->TypeId);
    TypeRegistry::ForEachBaseClass(typeId, [&](const TypeDescriptor& baseDesc)
    {
        RegisterServiceAs(registration, baseDesc.GetQualifiedName());
    });
}
