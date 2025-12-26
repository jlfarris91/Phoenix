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

ServiceRegistrar ServiceContainerBuilder::RegisterService(const TSharedPtr<IService>& service)
{
    const TypeDescriptor& typeDescriptor = service->GetTypeDescriptor();
    Container.ServiceMap.emplace(typeDescriptor.GetFName(), service);
    return { this, service };
}

TSharedPtr<ServiceContainer> ServiceContainerBuilder::Build()
{
    return MakeShared<ServiceContainer>(std::move(Container));
}

void ServiceContainerBuilder::RegisterServiceAs(const TSharedPtr<IService>& service, const FName& typeId)
{
    Container.ServiceAsMap[typeId].PushBackUnique(service);
}
