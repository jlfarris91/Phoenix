#include "ServiceContainer.h"

#include "Service.h"

using namespace Phoenix;

ServiceContainer::ServiceContainer(const std::shared_ptr<ServiceContainer>& parent)
    : Parent(parent)
{
}

std::shared_ptr<ServiceContainer> ServiceContainer::GetParent() const
{
    return Parent.lock();
}

std::shared_ptr<IService> ServiceContainer::ResolveService(const std::string& typeId)
{
    // Check if an existing instance already exists for the type id
    if (auto existingService = GetService(typeId))
    {
        return existingService;
    }

    // Find the registration(s) for the type id, starting with this service container and then searching parent service containers if necessary.
    std::vector<std::shared_ptr<const ServiceRegistration>> registrations;
    if (FindRegistrationsForTypeId(typeId, registrations) == 0)
    {
        return {};
    }

    std::shared_ptr<const ServiceRegistration> firstRegistration = registrations.front();

    auto parentContainer = Parent.lock();

    // If the registration is instance per scope, then we need to create a new instance in this container.
    // Otherwise, we create it in the parent container.
    if (parentContainer && !firstRegistration->InstancePerScope)
    {
        return parentContainer->ResolveService(typeId);
    }

    auto instance = firstRegistration->FactoryFunc(shared_from_this());
    if (!instance)
    {
        return {};
    }

    Instances.push_back(instance);
    TypeIdToInstanceMap[typeId] = instance;

    for (const auto& baseId : firstRegistration->BaseIds)
    {
        BaseIdToInstancesMap[baseId].push_back(instance);
    }

    return instance;
}

std::shared_ptr<IService> ServiceContainer::GetService(const std::string& typeId) const
{
    // Check if an existing instance already exists for the type id
    {
        auto existingInstanceIter = TypeIdToInstanceMap.find(typeId);
        if (existingInstanceIter != TypeIdToInstanceMap.end())
        {
            return existingInstanceIter->second;
        }
    }

    // Check if an existing instance already exists for the type id as a base type
    {
        auto existingInstanceIter = BaseIdToInstancesMap.find(typeId);
        if (existingInstanceIter != BaseIdToInstancesMap.end())
        {
            return existingInstanceIter->second.front();
        }
    }

    return {};
}

uint32_t ServiceContainer::ResolveServices(
    const std::string& typeId,
    std::vector<std::shared_ptr<IService>>& outServices)
{
    std::vector<std::shared_ptr<const ServiceRegistration>> registrations;
    FindRegistrationsForTypeId(typeId, registrations);

    uint32_t count = 0;
    for (const auto& registration : registrations)
    {
        if (auto instance = ResolveService(registration->TypeId))
        {
            outServices.push_back(instance);
            ++count;
        }
    }

    return count;
}

uint32_t ServiceContainer::GetServices(const std::string& typeId, std::vector<std::shared_ptr<IService>>& outServices) const
{
    std::vector<std::shared_ptr<const ServiceRegistration>> registrations;
    FindRegistrationsForTypeId(typeId, registrations);

    uint32_t count = 0;
    for (const auto& registration : registrations)
    {
        if (auto instance = GetService(registration->TypeId))
        {
            outServices.push_back(instance);
            ++count;
        }
    }

    return count;
}

uint32_t ServiceContainer::GetAllServices(std::vector<std::shared_ptr<IService>>& outServices) const
{
    uint32_t count = 0;

    for (const auto& service : Instances)
    {
        outServices.push_back(service);
        ++count;
    }

    if (auto parent = Parent.lock())
    {
        std::vector<std::shared_ptr<IService>> parentServices;
        parent->GetAllServices(parentServices);

        for (const std::shared_ptr<IService>& parentService : parentServices)
        {
            if (std::ranges::find(outServices, parentService) == outServices.end())
            {
                outServices.push_back(parentService);
                ++count;
            }
        }
    }

    return count;
}

const std::unordered_map<std::string, std::shared_ptr<IService>>& ServiceContainer::GetServiceMap() const
{
    return TypeIdToInstanceMap;
}

const std::unordered_map<std::string, std::vector<std::shared_ptr<IService>>>& ServiceContainer::GetServiceAsMap() const
{
    return BaseIdToInstancesMap;
}

uint32_t ServiceContainer::FindRegistrationsForTypeId(
    const std::string& typeId,
    std::vector<std::shared_ptr<const ServiceRegistration>>& outRegistrations) const
{
    uint32_t count = 0;

    // Look for direct match first
    auto iter = TypeIdToRegistrationMap.find(typeId);
    if (iter != TypeIdToRegistrationMap.end())
    {
        outRegistrations.push_back(iter->second);
        ++count;
    }

    // Then include any registrations for the type
    auto iter2 = BaseIdToRegistrationsMap.find(typeId);
    if (iter2 != BaseIdToRegistrationsMap.end())
    {
        for (const auto& registration : iter2->second)
        {
            outRegistrations.push_back(registration);
            ++count;
        }
    }

    // Then include any registrations from the parent locator that are not already captured
    if (auto parent = Parent.lock())
    {
        std::vector<std::shared_ptr<const ServiceRegistration>> parentRegistrations;
        parent->FindRegistrationsForTypeId(typeId, parentRegistrations);

        for (const auto& parentRegistration : parentRegistrations)
        {
            if (std::ranges::find(outRegistrations, parentRegistration) == outRegistrations.end())
            {
                outRegistrations.push_back(parentRegistration);
                ++count;
            }
        }
    }

    return count;
}

const std::shared_ptr<ServiceContainer>& ServiceContainerOwner::GetServiceContainer() const
{
    return Container;
}

std::shared_ptr<IService> ServiceContainerOwner::ResolveService(const std::string& typeId)
{
    return Container->ResolveService(typeId);
}

uint32_t ServiceContainerOwner::ResolveServices(const std::string& typeId, std::vector<std::shared_ptr<IService>>& outServices)
{
    return Container->ResolveServices(typeId, outServices);
}

std::shared_ptr<IService> ServiceContainerOwner::GetService(const std::string& typeId) const
{
    return Container->ResolveService(typeId);
}

uint32_t ServiceContainerOwner::GetServices(const std::string& typeId, std::vector<std::shared_ptr<IService>>& outServices) const
{
    return Container->ResolveServices(typeId, outServices);
}

uint32_t ServiceContainerOwner::GetAllServices(std::vector<std::shared_ptr<IService>>& outServices) const
{
    return Container->GetAllServices(outServices);
}