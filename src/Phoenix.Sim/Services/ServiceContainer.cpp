#include "Phoenix.Sim/Services/ServiceContainer.h"

#include "Service.h"

using namespace Phoenix;

std::shared_ptr<IService> ServiceContainer::GetService(const FName& typeId) const
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

uint32 ServiceContainer::GetServices(const FName& typeId, std::vector<std::shared_ptr<IService>>& outServices) const
{
    uint32 count = 0;

    // Look for direct match first
    if (std::shared_ptr<IService> service = GetService(typeId))
    {
        outServices.push_back(service);
        ++count;
    }

    // Then include any services registered as the type
    auto iter = ServiceAsMap.find(typeId);
    if (iter != ServiceAsMap.end())
    {
        for (const std::shared_ptr<IService>& service : iter->second)
        {
            outServices.push_back(service);
            ++count;
        }
    }

    return count;
}

const std::vector<std::shared_ptr<IService>>& ServiceContainer::GetServices() const
{
    return Services;
}

const std::unordered_map<FName, std::shared_ptr<IService>>& ServiceContainer::GetServiceMap() const
{
    return ServiceMap;
}

const std::unordered_map<FName, std::vector<std::shared_ptr<IService>>>& ServiceContainer::GetServiceAsMap() const
{
    return ServiceAsMap;
}
