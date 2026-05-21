#include "ServiceContainer.h"

#include "IService.h"

using namespace Phoenix;

ServiceContainer::ServiceContainer(std::shared_ptr<ServiceContainer> parent)
    : Parent(std::move(parent))
{
}

std::shared_ptr<ServiceContainer> ServiceContainer::GetParent() const
{
    return Parent.lock();
}

std::shared_ptr<IService> ServiceContainer::ResolveService(FName typeId)
{
    if (auto existing = GetService(typeId))
        return existing;

    std::vector<std::shared_ptr<const ServiceRegistration>> registrations;
    if (FindRegistrations(typeId, registrations) == 0)
        return {};

    const auto& firstRegistration = registrations.front();
    auto parentContainer = Parent.lock();

    if (parentContainer && !firstRegistration->InstancePerScope)
        return parentContainer->ResolveService(typeId);

    auto instance = firstRegistration->FactoryFunc(*this);
    if (!instance)
        return {};

    Instances.push_back(instance);
    TypeIdToInstance[firstRegistration->TypeId] = instance;

    for (const FName& baseId : firstRegistration->BaseIds)
        BaseIdToInstances[baseId].push_back(instance);

    return instance;
}

std::shared_ptr<IService> ServiceContainer::GetService(FName typeId) const
{
    auto it = TypeIdToInstance.find(typeId);
    if (it != TypeIdToInstance.end())
        return it->second;

    auto it2 = BaseIdToInstances.find(typeId);
    if (it2 != BaseIdToInstances.end())
        return it2->second.front();

    return {};
}

uint32_t ServiceContainer::ResolveServices(FName typeId, std::vector<std::shared_ptr<IService>>& out)
{
    std::vector<std::shared_ptr<const ServiceRegistration>> registrations;
    FindRegistrations(typeId, registrations);

    uint32_t count = 0;
    for (const auto& reg : registrations)
    {
        if (auto instance = ResolveService(reg->TypeId))
        {
            out.push_back(instance);
            ++count;
        }
    }
    return count;
}

uint32_t ServiceContainer::GetServices(FName typeId, std::vector<std::shared_ptr<IService>>& out) const
{
    std::vector<std::shared_ptr<const ServiceRegistration>> registrations;
    FindRegistrations(typeId, registrations);

    uint32_t count = 0;
    for (const auto& reg : registrations)
    {
        if (auto instance = GetService(reg->TypeId))
        {
            out.push_back(instance);
            ++count;
        }
    }
    return count;
}

uint32_t ServiceContainer::GetAllServices(std::vector<std::shared_ptr<IService>>& out) const
{
    uint32_t count = 0;
    for (const auto& service : Instances)
    {
        out.push_back(service);
        ++count;
    }

    if (auto parent = Parent.lock())
    {
        std::vector<std::shared_ptr<IService>> parentServices;
        parent->GetAllServices(parentServices);

        for (const auto& parentService : parentServices)
        {
            if (std::ranges::find(out, parentService) == out.end())
            {
                out.push_back(parentService);
                ++count;
            }
        }
    }

    return count;
}

std::vector<std::shared_ptr<IService>> ServiceContainer::GetInstances() const
{
    return Instances;
}

const std::unordered_map<FName, std::shared_ptr<IService>>& ServiceContainer::GetServiceMap() const
{
    return TypeIdToInstance;
}

const std::unordered_map<FName, std::vector<std::shared_ptr<IService>>>& ServiceContainer::GetServiceAsMap() const
{
    return BaseIdToInstances;
}

uint32_t ServiceContainer::FindRegistrations(
    FName typeId,
    std::vector<std::shared_ptr<const ServiceRegistration>>& out) const
{
    uint32_t count = 0;

    auto it = TypeIdToRegistration.find(typeId);
    if (it != TypeIdToRegistration.end())
    {
        out.push_back(it->second);
        ++count;
    }

    auto it2 = BaseIdToRegistrations.find(typeId);
    if (it2 != BaseIdToRegistrations.end())
    {
        for (const auto& reg : it2->second)
        {
            out.push_back(reg);
            ++count;
        }
    }

    if (auto parent = Parent.lock())
    {
        std::vector<std::shared_ptr<const ServiceRegistration>> parentRegs;
        parent->FindRegistrations(typeId, parentRegs);

        for (const auto& parentReg : parentRegs)
        {
            if (std::ranges::find(out, parentReg) == out.end())
            {
                out.push_back(parentReg);
                ++count;
            }
        }
    }

    return count;
}

// ServiceContainerOwner

const std::shared_ptr<ServiceContainer>& ServiceContainerOwner::GetServiceContainer() const
{
    return Container;
}

std::shared_ptr<IService> ServiceContainerOwner::ResolveService(FName typeId)
{
    return Container->ResolveService(typeId);
}

uint32_t ServiceContainerOwner::ResolveServices(FName typeId, std::vector<std::shared_ptr<IService>>& out)
{
    return Container->ResolveServices(typeId, out);
}

std::shared_ptr<IService> ServiceContainerOwner::GetService(FName typeId) const
{
    return Container->GetService(typeId);
}

uint32_t ServiceContainerOwner::GetServices(FName typeId, std::vector<std::shared_ptr<IService>>& out) const
{
    return Container->GetServices(typeId, out);
}

uint32_t ServiceContainerOwner::GetAllServices(std::vector<std::shared_ptr<IService>>& out) const
{
    return Container->GetAllServices(out);
}
