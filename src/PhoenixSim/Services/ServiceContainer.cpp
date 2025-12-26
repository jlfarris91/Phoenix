#include "PhoenixSim/Services/ServiceContainer.h"

#include "Service.h"

using namespace Phoenix;

TSharedPtr<IService> ServiceContainer::GetService(const FName& typeId) const
{
    auto iter = ServiceMap.find(typeId);
    return iter != ServiceMap.end() ? iter->second : nullptr;
}

uint32 ServiceContainer::GetServices(const FName& typeId, TArray2<TSharedPtr<IService>>& outServices) const
{
    uint32 count = 0;

    // Look for direct match first
    if (TSharedPtr<IService> service = GetService(typeId))
    {
        outServices.PushBack(service);
        ++count;
    }

    // Then include any services registered as the type
    auto iter = ServiceAsMap.find(typeId);
    if (iter != ServiceAsMap.end())
    {
        for (const TSharedPtr<IService>& service : iter->second)
        {
            outServices.PushBack(service);
            ++count;
        }
    }

    return count;
}

const TArray2<TSharedPtr<IService>>& ServiceContainer::GetServices() const
{
    return Services;
}

const TMap<FName, std::shared_ptr<IService>>& ServiceContainer::GetServiceMap() const
{
    return ServiceMap;
}

const TMap<FName, TArray2<TSharedPtr<IService>>>& ServiceContainer::GetServiceAsMap() const
{
    return ServiceAsMap;
}
