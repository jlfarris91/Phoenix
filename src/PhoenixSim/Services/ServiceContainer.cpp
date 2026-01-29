#include "PhoenixSim/Services/ServiceContainer.h"

#include "Service.h"

using namespace Phoenix;

TSharedPtr<IService> ServiceContainer::GetService(const FName& typeId) const
{
    auto iter = ServiceMap.find(typeId);
    if (iter != ServiceMap.end())
    {
        return iter->second;
    }
    auto iter2 = ServiceAsMap.find(typeId);
    if (iter2 != ServiceAsMap.end())
    {
        return iter2->second.front();
    }
    return nullptr;
}

uint32 ServiceContainer::GetServices(const FName& typeId, TVector<TSharedPtr<IService>>& outServices) const
{
    uint32 count = 0;

    // Look for direct match first
    if (TSharedPtr<IService> service = GetService(typeId))
    {
        outServices.push_back(service);
        ++count;
    }

    // Then include any services registered as the type
    auto iter = ServiceAsMap.find(typeId);
    if (iter != ServiceAsMap.end())
    {
        for (const TSharedPtr<IService>& service : iter->second)
        {
            outServices.push_back(service);
            ++count;
        }
    }

    return count;
}

const TVector<TSharedPtr<IService>>& ServiceContainer::GetServices() const
{
    return Services;
}

const std::unordered_map<FName, std::shared_ptr<IService>>& ServiceContainer::GetServiceMap() const
{
    return ServiceMap;
}

const std::unordered_map<FName, TVector<TSharedPtr<IService>>>& ServiceContainer::GetServiceAsMap() const
{
    return ServiceAsMap;
}
