#include "ArchetypeManager.h"

#include <cstring>

using namespace Phoenix;
using namespace Phoenix::ECS;

uint32 ArchetypeManager::GetAllocSizeBytes(const Config& config)
{
    uint32 allocSize = 0;
    allocSize += TComponentDefMap::GetAllocSizeBytes(config.MaxComponentDefs);
    allocSize += TArchetypeDefMap::GetAllocSizeBytes(config.MaxArchetypeDefs);
    allocSize += TEntityHandleMap::GetAllocSizeBytes(config.MaxEntities);

    FixedBlockAllocator::Config allocatorConfig;
    allocatorConfig.BlockSize = config.ArchetypeListSize + sizeof(TArchetypeList);
    allocatorConfig.Capacity = config.MaxArchetypeLists;
    allocSize += TArchetypeListAllocator::GetAllocSizeBytes(allocatorConfig);

    return allocSize;
}

uint32 ArchetypeManager::GetAllocSizeBytes() const
{
    return GetAllocSizeBytes(Configuration);
}

bool ArchetypeManager::IsValid(TArchetypeHandle handle) const
{
    const TArchetypeList* list = FindOwningArchetypeList(handle);
    return list && list->IsValid(handle);
}

uint32 ArchetypeManager::GetNumActiveArchetypes() const
{
    uint32 total = 0;
    for (const TBlockHandle& handle : ArchetypeLists)
    {
        if (const TArchetypeList* list = ArchetypeLists.GetPtr<TArchetypeList>(handle))
        {
            total += list->GetNumActiveInstances();
        }
    }
    return total;
}

uint32 ArchetypeManager::GetNumArchetypeLists() const
{
    return ArchetypeLists.GetNumOccupiedBlocks();
}

const ArchetypeManager::TArchetypeDefMap& ArchetypeManager::GetArchetypeDefinitions() const
{
    return ArchetypeDefinitions;
}

bool ArchetypeManager::RegisterArchetypeDefinition(const ArchetypeDefinition& definition)
{
    if (ArchetypeDefinitions.Contains(definition.GetArchetypeHash()))
    {
        return true;
    }

    if (FName::IsNoneOrEmpty(definition.GetArchetypeHash()))
    {
        return false;
    }

    if (ArchetypeDefinitions.IsFull())
    {
        return false;
    }

    for (uint16 i = 0; i < definition.GetNumComponents(); ++i)
    {
        if (!RegisterComponentDefinition(definition[i]))
        {
            return false;
        }
    }

    return ArchetypeDefinitions.Insert(definition.GetArchetypeHash(), definition);
}

bool ArchetypeManager::UnregisterArchetypeDefinition(const ArchetypeDefinition& definition)
{
    // TODO (jfarris): what should we do with existing archetype lists?
    return ArchetypeDefinitions.Remove(definition.GetArchetypeHash());
}

bool ArchetypeManager::IsArchetypeRegistered(const FName& archetypeIdOrHash) const
{
    return GetArchetypeDefinition(archetypeIdOrHash) != nullptr;
}

FName ArchetypeManager::GetArchetypeHash(const FName& archetypeId) const
{
    const ArchetypeDefinition* archDefPtr = GetArchetypeDefinitionById(archetypeId);
    return archDefPtr ? FName(archDefPtr->GetArchetypeHash()) : FName::None;
}

const ArchetypeDefinition* ArchetypeManager::GetArchetypeDefinitionByHash(const FName& archetypeHash) const
{
    return ArchetypeDefinitions.GetPtr(archetypeHash);
}

const ArchetypeDefinition* ArchetypeManager::GetArchetypeDefinitionById(const FName& archetypeId) const
{
    for (auto && [compHash, archDef] : ArchetypeDefinitions)
    {
        if (archDef.GetId() == archetypeId)
            return &archDef;
    }
    return nullptr;
}

const ArchetypeDefinition* ArchetypeManager::GetArchetypeDefinition(const FName& archetypeIdOrHash) const
{
    if (const ArchetypeDefinition* archDefPtr = GetArchetypeDefinitionByHash(archetypeIdOrHash))
    {
        return archDefPtr;
    }
    return GetArchetypeDefinitionById(archetypeIdOrHash);
}

const ArchetypeManager::TComponentDefMap& ArchetypeManager::GetComponentDefinitions() const
{
    return ComponentDefinitions;
}

bool ArchetypeManager::RegisterComponentDefinition(const ComponentDefinition& definition)
{
    if (IsComponentRegistered(definition.Id))
        return true;

    if (ComponentDefinitions.IsFull())
        return false;

    return ComponentDefinitions.Insert(definition.Id, definition);
}

bool ArchetypeManager::UnregisterComponentDefinition(const FName& componentId)
{
    // TODO (jfarris): what should we do with existing archetype definitions containing the component?
    return ComponentDefinitions.Remove(componentId);
}

bool ArchetypeManager::IsComponentRegistered(const FName& componentId) const
{
    return ComponentDefinitions.Contains(componentId);
}

ArchetypeManager::TArchetypeHandle ArchetypeManager::Acquire(EntityId entityId, const FName& archetypeIdOrHash)
{
    TArchetypeList* list = FindOrAddArchetypeList(archetypeIdOrHash);
    if (!list)
    {
        return {};
    }

    TArchetypeHandle handle = list->Acquire(entityId);
    if (handle != TArchetypeHandle{})
    {
        EntityHandles.Insert(handle.GetEntityId(), handle);
    }

    return handle;
}

bool ArchetypeManager::Release(const TArchetypeHandle& handle)
{
    TArchetypeList* list = FindOwningArchetypeList(handle);
    if (!list)
    {
        return false;
    }

    // TODO (jfarris): release chunk when list becomes empty?
    if (!list->Release(handle))
    {
        return false;
    }

    EntityHandles.Remove(handle.GetEntityId());
    return true;
}

bool ArchetypeManager::Release(EntityId entityId)
{
    TArchetypeHandle handle = GetHandleForEntity(entityId);
    return Release(handle);
}

ArchetypeManager::TArchetypeHandle ArchetypeManager::SetArchetype(
    TArchetypeHandle& inOutHandle,
    const FName& archetypeIdOrHash)
{
    EntityId entityId = inOutHandle.GetEntityId();
    if (entityId == EntityId::Invalid)
    {
        return inOutHandle;
    }

    TArchetypeList::Handle newHandle(entityId);

    TArchetypeList* currList = FindOwningArchetypeList(inOutHandle);

    // The entity is already in a list with the given archetype id. Nothing changes.
    if (currList && currList->HasArchetypeDefinition(archetypeIdOrHash))
    {
        return inOutHandle;
    }

    // Allocate a new archetype for the entity in a new list.
    if (TArchetypeList* newList = FindOrAddArchetypeList(archetypeIdOrHash))
    {
        newHandle = newList->Acquire(entityId);

        // Copy over any component data to the new archetype
        if (currList)
        {
            currList->ForEachComponent(inOutHandle, [&](const ComponentDefinition& compDef, const void* currComp)
            {
                if (void* newComp = newList->GetComponent(newHandle, compDef.Id))
                {
                    memcpy(newComp, currComp, compDef.Size);
                }
            });
        }
    }

    // Try to remove the entity from the list it currently belongs to.
    if (currList)
    {
        currList->Release(inOutHandle);
    }

    EntityHandles.Insert(entityId, newHandle);
    inOutHandle = newHandle;

    return newHandle;
}

ArchetypeManager::TArchetypeHandle ArchetypeManager::SetArchetype(EntityId entityId, const FName& archetypeIdOrHash)
{
    TArchetypeHandle handle = GetHandleForEntity(entityId);
    return SetArchetype(handle, archetypeIdOrHash);
}

void* ArchetypeManager::AddComponent(TArchetypeHandle& inOutHandle, const ComponentDefinition& componentDef)
{
    ArchetypeDefinition currArchDef;

    if (const TArchetypeList* currList = FindOwningArchetypeList(inOutHandle))
    {
        currArchDef = currList->GetDefinition();
    }

    ArchetypeDefinition newArchDef;
    if (!ArchetypeDefinition::AddComponent(currArchDef, componentDef, newArchDef))
    {
        // Failed to add the component to the archetype.
        // Could be that the current archetype definition already had that component, or it can't add any more.
        return nullptr;
    }

    ArchetypeDefinition* archDef = ArchetypeDefinitions.FindOrAdd(newArchDef.GetArchetypeHash(), newArchDef);
    if (!archDef)
    {
        // Failed to add the new archetype, probably out of space.
        return nullptr;
    }

    SetArchetype(inOutHandle, archDef->GetArchetypeHash());

    return GetComponent(inOutHandle, componentDef.Id);
}

void* ArchetypeManager::AddComponent(EntityId entityId, const ComponentDefinition& componentDef)
{
    TArchetypeHandle handle = GetHandleForEntity(entityId);
    return AddComponent(handle, componentDef);
}

void* ArchetypeManager::AddComponent(TArchetypeHandle& inOutHandle, const FName& componentId)
{
    const ComponentDefinition* compDef = ComponentDefinitions.GetPtr(componentId);
    if (!compDef)
    {
        return nullptr;
    }

    return AddComponent(inOutHandle, *compDef);
}

void* ArchetypeManager::AddComponent(EntityId entityId, const FName& componentId)
{
    TArchetypeHandle handle = GetHandleForEntity(entityId);
    return AddComponent(handle, componentId);
}

bool ArchetypeManager::RemoveComponent(TArchetypeHandle& inOutHandle, const FName& componentId)
{
    ArchetypeDefinition currArchDef;

    const TArchetypeList* currList = FindOwningArchetypeList(inOutHandle);
    if (!currList)
    {
        return false;
    }

    ArchetypeDefinition newArchDef;
    if (!ArchetypeDefinition::RemoveComponent(currArchDef, componentId, newArchDef))
    {
        // Failed to remove the component from the archetype.
        // The current archetype definition must not have the component.
        return false;
    }

    ArchetypeDefinition* archDef = ArchetypeDefinitions.FindOrAdd(newArchDef.GetArchetypeHash(), newArchDef);
    if (!archDef)
    {
        // Failed to add the new archetype, probably out of space.
        return false;
    }

    inOutHandle = SetArchetype(inOutHandle, archDef->GetArchetypeHash());

    return true;
}

bool ArchetypeManager::RemoveComponent(EntityId entityId, const FName& componentId)
{
    TArchetypeHandle handle = GetHandleForEntity(entityId);
    if (RemoveComponent(handle, componentId))
    {
        EntityHandles.Insert(entityId, handle);
        return true;
    }
    return false;
}

bool ArchetypeManager::RemoveAllComponents(const TArchetypeHandle& handle)
{
    TArchetypeList* currList = FindOwningArchetypeList(handle);
    return currList && currList->Release(handle);
}

bool ArchetypeManager::RemoveAllComponents(EntityId entityId)
{
    TArchetypeHandle handle = GetHandleForEntity(entityId);
    return RemoveAllComponents(handle);
}

void* ArchetypeManager::GetComponent(const TArchetypeHandle& handle, const FName& componentId)
{
    PHX_PROFILE_ZONE_SCOPED;

    TArchetypeList* list = FindOwningArchetypeList(handle);
    if (!list || !list->IsValid(handle))
    {
        return nullptr;
    }

    return list->GetComponent(handle, componentId);
}

const void* ArchetypeManager::GetComponent(const TArchetypeHandle& handle, const FName& componentId) const
{
    PHX_PROFILE_ZONE_SCOPED;

    const TArchetypeList* list = FindOwningArchetypeList(handle);
    if (!list || !list->IsValid(handle))
    {
        return nullptr;
    }

    return list->GetComponent(handle, componentId);
}

void* ArchetypeManager::GetComponent(EntityId entityId, const FName& componentId)
{
    TArchetypeHandle handle = GetHandleForEntity(entityId);
    return GetComponent(handle, componentId);
}

const void* ArchetypeManager::GetComponent(EntityId entityId, const FName& componentId) const
{
    TArchetypeHandle handle = GetHandleForEntity(entityId);
    return GetComponent(handle, componentId);
}

ArchetypeManager::TArchetypeList* ArchetypeManager::FindFirstArchetypeList(
    const FName& archetypeIdOrHash,
    bool includeFullLists)
{
    for (const TBlockHandle& handle : ArchetypeLists)
    {
        TArchetypeList* list = ArchetypeLists.GetPtr<TArchetypeList>(handle);
        if (list && list->HasArchetypeDefinition(archetypeIdOrHash) && (includeFullLists || !list->IsFull()))
        {
            return list;
        }
    }
    return nullptr;
}

ArchetypeManager::TArchetypeList* ArchetypeManager::FindOwningArchetypeList(const TArchetypeHandle& handle)
{
    PHX_PROFILE_ZONE_SCOPED;
    TArchetypeList* list = ArchetypeLists.GetPtr<TArchetypeList>(TBlockHandle { handle.GetOwnerId() });
    return list && list->IsValid(handle) ? list : nullptr;
}

const ArchetypeManager::TArchetypeList* ArchetypeManager::FindOwningArchetypeList(const TArchetypeHandle& handle) const
{
    PHX_PROFILE_ZONE_SCOPED;
    const TArchetypeList* list = ArchetypeLists.GetPtr<TArchetypeList>(TBlockHandle { handle.GetOwnerId() });
    return list && list->IsValid(handle) ? list : nullptr;
}

ArchetypeManager::TArchetypeList* ArchetypeManager::FindOwningArchetypeList(EntityId entityId)
{
    TArchetypeHandle handle = GetHandleForEntity(entityId);
    return FindOwningArchetypeList(handle);
}

const ArchetypeManager::TArchetypeList* ArchetypeManager::FindOwningArchetypeList(EntityId entityId) const
{
    TArchetypeHandle handle = GetHandleForEntity(entityId);
    return FindOwningArchetypeList(handle);
}

void ArchetypeManager::ForEachArchetypeList(const std::function<void(TArchetypeList&)>& func)
{
    PHX_PROFILE_ZONE_SCOPED;
    for (const TBlockHandle& handle : ArchetypeLists)
    {
        TArchetypeList* list = ArchetypeLists.GetPtr<TArchetypeList>(handle);
        if (list && InvokeForEachCallbackNoIndex(func, *list))
        {
            break;
        }
    }
}

void ArchetypeManager::ForEachArchetypeList(const std::function<void(const TArchetypeList&)>& func) const
{
    PHX_PROFILE_ZONE_SCOPED;
    for (const TBlockHandle& handle : ArchetypeLists)
    {
        const TArchetypeList* list = ArchetypeLists.GetPtr<TArchetypeList>(handle);
        if (list && InvokeForEachCallbackNoIndex(func, *list))
        {
            break;
        }
    }
}

void ArchetypeManager::ForEachArchetypeList(const FName& archetypeIdOrHash,
    const std::function<void(TArchetypeList&)>& func)
{
    PHX_PROFILE_ZONE_SCOPED;
    for (const TBlockHandle& handle : ArchetypeLists)
    {
        TArchetypeList* list = ArchetypeLists.GetPtr<TArchetypeList>(handle);
        if (list && list->HasArchetypeDefinition(archetypeIdOrHash) && InvokeForEachCallbackNoIndex(func, *list))
        {
            break;
        }
    }
}

void ArchetypeManager::Compact()
{
    // Free any archetype lists with no active instances.
    for (const TBlockHandle& handle : ArchetypeLists)
    {
        TArchetypeList* list = ArchetypeLists.GetPtr<TArchetypeList>(handle);
        if (list && list->GetNumActiveInstances() == 0)
        {
            ArchetypeLists.Deallocate(handle);
        }
    }

    ArchetypeLists.Compact();
}

ArchetypeManager::TArchetypeHandle ArchetypeManager::GetHandleForEntity(EntityId entityId) const
{
    const TArchetypeHandle* handlePtr = EntityHandles.GetPtr(entityId);
    return handlePtr ? *handlePtr : ArchetypeHandle(entityId);
}

ArchetypeManager::TArchetypeList* ArchetypeManager::FindOrAddArchetypeList(const FName& archetypeIdOrHash)
{
    TArchetypeList* list = FindFirstArchetypeList(archetypeIdOrHash, false);
    if (list)
    {
        return list;
    }

    const ArchetypeDefinition* archetypeDef = GetArchetypeDefinition(archetypeIdOrHash);
    if (!archetypeDef)
    {
        return nullptr;
    }

    TBlockHandle handle = ArchetypeLists.Allocate<TArchetypeList>(archetypeDef->GetArchetypeHash(), *archetypeDef);

    list = ArchetypeLists.GetPtr<TArchetypeList>(handle);
    if (list)
    {
        list->SetId(handle.Id);
    }

    return list;
}
