#include "PhoenixSim/Services/ServiceContainerBuilder.h"

#include "ServiceContainer.h"

using namespace Phoenix;

ServiceRegistrar::ServiceRegistrar(ServiceContainerBuilder* builder, const std::shared_ptr<IService>& service)
    : Builder(builder)
    , Service(service)
{
}

const ServiceRegistrar& ServiceRegistrar::As(const FName& typeId) const
{
    Builder->RegisterServiceAs(Service, typeId);
    return *this;
}

const ServiceRegistrar& ServiceRegistrar::AsInterfaces() const
{
    Builder->RegisterServiceAsInterfaces(Service);
    return *this;
}

ServiceRegistrar ServiceContainerBuilder::RegisterService(const std::shared_ptr<IService>& service)
{
    const TypeDescriptor& typeDescriptor = service->GetTypeDescriptor();
    Container.Services.push_back(service);
    Container.ServiceMap.emplace(typeDescriptor.GetTypeId(), service);
    return { this, service };
}

std::shared_ptr<ServiceContainer> ServiceContainerBuilder::Build()
{
    return std::make_shared<ServiceContainer>(std::move(Container));
}

void ServiceContainerBuilder::RegisterServiceAs(const std::shared_ptr<IService>& service, const FName& typeId)
{
    Container.ServiceAsMap[typeId].push_back(service);
}

void ServiceContainerBuilder::RegisterServiceAsInterfaces(const std::shared_ptr<IService>& service)
{
    TypeRegistry::ForEachBaseClass(service->GetTypeDescriptor().GetTypeId(), [&](const TypeDescriptor& interface)
    {
        RegisterServiceAs(service, interface.GetTypeId());
    });
}
