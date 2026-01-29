#include "PhoenixSim/Services/ServiceContainerBuilder.h"

#include "ServiceContainer.h"

using namespace Phoenix;

ServiceRegistrar::ServiceRegistrar(ServiceContainerBuilder* builder, const TSharedPtr<IService>& service)
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

ServiceRegistrar ServiceContainerBuilder::RegisterService(const TSharedPtr<IService>& service)
{
    const TypeDescriptor& typeDescriptor = service->GetTypeDescriptor();
    Container.Services.push_back(service);
    Container.ServiceMap.emplace(typeDescriptor.GetFName(), service);
    return { this, service };
}

TSharedPtr<ServiceContainer> ServiceContainerBuilder::Build()
{
    return MakeShared<ServiceContainer>(std::move(Container));
}

void ServiceContainerBuilder::RegisterServiceAs(const TSharedPtr<IService>& service, const FName& typeId)
{
    Container.ServiceAsMap[typeId].push_back(service);
}

void ServiceContainerBuilder::RegisterServiceAsInterfaces(const TSharedPtr<IService>& service)
{
    service->GetTypeDescriptor().ForEachInterface([&](const TypeDescriptor& interface)
    {
        RegisterServiceAs(service, interface.FName);
    });
}
