#include "ArchetypeList.h"

#include "PhoenixSim/Profiling.h"

using namespace Phoenix;
using namespace Phoenix::ECS;

uint32 FixedArchetypeList::GetAllocSizeBytes(uint32 capacity)
{
    return capacity;
}

uint32 FixedArchetypeList::GetAllocSizeBytes() const
{
    return GetAllocSizeBytes(GetCapacity());
}

uint32 FixedArchetypeList::GetCapacity() const
{
    return Capacity;
}

uint32 FixedArchetypeList::GetId() const
{
    return Id;
}

void FixedArchetypeList::SetId(uint32 id)
{
    Id = id;
}

const ArchetypeDefinition& FixedArchetypeList::GetDefinition() const
{
    return Definition;
}

bool FixedArchetypeList::HasArchetypeDefinition(const FName& archetypeIdOrHash) const
{
    return Definition.HasIdOrHash(archetypeIdOrHash);
}

void* FixedArchetypeList::GetData()
{
    return Data.GetData();
}

const void* FixedArchetypeList::GetData() const
{
    return Data.GetData();
}

uint32 FixedArchetypeList::GetSize() const
{
    return NumInstances * GetEntityTotalSize();
}

bool FixedArchetypeList::IsFull() const
{
    return NumActiveInstances == GetInstanceCapacity();
}

uint32 FixedArchetypeList::GetNumInstances() const
{
    return NumInstances;
}

uint32 FixedArchetypeList::GetNumActiveInstances() const
{
    return NumActiveInstances;
}

uint32 FixedArchetypeList::GetInstanceCapacity() const
{
    return Capacity / GetEntityTotalSize();
}

bool FixedArchetypeList::IsValid(const Handle& handle) const
{
    if (!OwnsHandle(handle) || handle.Id >= NumInstances)
    {
        return false;
    }

    const ArchetypeInstance* instance = GetEntityPtr(handle.Id);
    return instance && instance->EntityId == handle.EntityId;
}

bool FixedArchetypeList::OwnsHandle(const Handle& handle) const
{
    return handle.OwnerId == Id;
}

FixedArchetypeList::Handle FixedArchetypeList::Acquire(EntityId entityId)
{
    if (IsFull())
    {
        return Handle();
    }

    uint32 slotIndex = FindFreeSlot();
    if (slotIndex == Index<uint32>::None)
    {
        slotIndex = NumInstances++;
    }

    Handle handle = { Id, slotIndex, entityId };
    ArchetypeInstance* instance = GetEntityPtr(handle.Id);
    instance->EntityId = entityId;

    void* entityComponentDataPtr = GetEntityComponentHeadPtr(handle.Id);
    PHX_ASSERT(entityComponentDataPtr);

    Definition.Construct(entityComponentDataPtr);

    ++NumActiveInstances;

    return handle;
}

bool FixedArchetypeList::Release(const Handle& handle)
{
    if (!OwnsHandle(handle))
    {
        return false;
    }

    ArchetypeInstance* instance = GetEntityPtr(handle.Id);
    if (!instance || instance->EntityId != handle.EntityId)
    {
        return false;
    }

    instance->EntityId = EntityId::Invalid;

    void* entityComponentDataPtr = GetEntityComponentHeadPtr(handle.Id);
    PHX_ASSERT(entityComponentDataPtr);

    Definition.Deconstruct(entityComponentDataPtr);

    --NumActiveInstances;

    if (handle.Id < FreeSlotIndexHint)
    {
        FreeSlotIndexHint = handle.Id;
    }

    return true;
}

void* FixedArchetypeList::GetComponent(const Handle& handle, const FName& componentId)
{
    PHX_PROFILE_ZONE_SCOPED;
    if (!IsValid(handle))
    {
        return nullptr;
    }
    uint32 entityCompOffset = GetOffsetToEntityComponent(handle.Id, componentId);
    if (entityCompOffset == Index<uint32>::None)
    {
        return nullptr;
    }
    return Data.GetData() + entityCompOffset;
}

const void* FixedArchetypeList::GetComponent(const Handle& handle, const FName& componentId) const
{
    PHX_PROFILE_ZONE_SCOPED;
    if (!IsValid(handle))
    {
        return nullptr;
    }
    uint32 entityCompOffset = GetOffsetToEntityComponent(handle.Id, componentId);
    if (entityCompOffset == Index<uint32>::None)
    {
        return nullptr;
    }
    return Data.GetData() + entityCompOffset;
}

uint32 FixedArchetypeList::GetEntityTotalSize() const
{
    return sizeof(ArchetypeInstance) + Definition.GetTotalSize();
}

uint32 FixedArchetypeList::GetComponentLocalOffset(const FName& componentId) const
{
    uint32 index = Definition.IndexOfComponent(componentId);
    if (!Definition.IsValidIndex(index))
    {
        return Index<uint32>::None;
    }
    return Definition[index].Offset;
}

void FixedArchetypeList::ForEachInstance(const TFunction<void(const Handle&)>& func) const
{
    for (uint32 i = 0; i < NumInstances; ++i)
    {
        const ArchetypeInstance* instance = GetEntityPtr(i);

        if (instance->EntityId == EntityId::Invalid)
        {
            continue;
        }

        if (InvokeForEachCallbackWithIndex(func, i, Handle(Id, i, instance->EntityId)))
        {
            break;
        }
    }
}

FixedArchetypeList::Handle FixedArchetypeList::GetFirstActiveEntity() const
{
    return GetNextActiveEntity({ Id, 0, 0 });
}

FixedArchetypeList::Handle FixedArchetypeList::GetNextActiveEntity(const Handle& handle) const
{
    if (!OwnsHandle(handle))
    {
        return {};
    }

    Handle result;
    for (uint32 i = handle.Id; i < NumInstances; ++i)
    {
        const ArchetypeInstance* instance = GetEntityPtr(i);

        if (instance->EntityId == EntityId::Invalid)
        {
            continue;
        }

        result = { Id, i, instance->EntityId };
        break;
    }

    return result;
}

template <class ... TComponents>
FixedArchetypeList::EntityComponentIter<TComponents...>::EntityComponentIter(FixedArchetypeList* list, const Handle& handle)
    : Curr(handle)
    , List(list)
{
    if (!List->IsValid(Curr))
        Curr = List->GetNextActiveEntity(Curr);
}

template <class ... TComponents>
TTuple<EntityId, TComponents...> FixedArchetypeList::EntityComponentIter<TComponents...>::operator*() const
{
    return std::make_tuple(Curr.EntityId, ComponentAccessor<TComponents>::template GetComponentRef(*this, Curr.Id)...);
}

template <class ... TComponents>
FixedArchetypeList::EntityComponentIter<TComponents...>& FixedArchetypeList::EntityComponentIter<TComponents...>::operator++()
{
    Curr = List->GetNextActiveEntity(Curr);
    return *this;
}

template <class ... TComponents>
bool FixedArchetypeList::EntityComponentIter<TComponents...>::operator==(const EntityComponentIter& other) const
{
    return List == other.List && Curr == other.Curr;
}

uint32 FixedArchetypeList::GetOffsetToEntity(uint32 index) const
{
    return uint32(index * GetEntityTotalSize());
}

uint32 FixedArchetypeList::GetOffsetToEntityComponentHead(uint32 index) const
{
    return GetOffsetToEntity(index) + sizeof(ArchetypeInstance);
}

uint32 FixedArchetypeList::GetOffsetToEntityComponent(uint32 index, const FName& componentId) const
{
    uint32 componentOffset = GetComponentLocalOffset(componentId);
    if (componentOffset == Index<uint32>::None)
    {
        return Index<uint32>::None;
    }
    return GetOffsetToEntityComponentHead(index) + componentOffset;
}

ArchetypeInstance* FixedArchetypeList::GetEntityPtr(uint32 index)
{
    uint32 offset = GetOffsetToEntity(index);
    if (offset == Index<uint32>::None)
    {
        return nullptr;
    }
    return reinterpret_cast<ArchetypeInstance*>(Data.GetData() + offset);
}

const ArchetypeInstance* FixedArchetypeList::GetEntityPtr(uint32 index) const
{
    uint32 offset = GetOffsetToEntity(index);
    if (offset == Index<uint32>::None)
    {
        return nullptr;
    }
    return reinterpret_cast<const ArchetypeInstance*>(Data.GetData() + offset);
}

void* FixedArchetypeList::GetEntityComponentHeadPtr(uint32 index)
{
    uint32 offset = GetOffsetToEntityComponentHead(index);
    if (offset == Index<uint32>::None)
    {
        return nullptr;
    }
    return Data.GetData() + offset;
}

const void* FixedArchetypeList::GetEntityComponentHeadPtr(uint32 index) const
{
    uint32 offset = GetOffsetToEntityComponentHead(index);
    if (offset == Index<uint32>::None)
    {
        return nullptr;
    }
    return Data.GetData() + offset;
}

void* FixedArchetypeList::GetEntityComponentPtr(uint32 index, const FName& componentId)
{
    uint32 offset = GetOffsetToEntityComponent(index, componentId);
    if (offset == Index<uint32>::None)
    {
        return nullptr;
    }
    return Data.GetData() + offset;
}

const void* FixedArchetypeList::GetEntityComponentPtr(uint32 index, const FName& componentId) const
{
    uint32 offset = GetOffsetToEntityComponent(index, componentId);
    if (offset == Index<uint32>::None)
    {
        return nullptr;
    }
    return Data.GetData() + offset;
}

uint32 FixedArchetypeList::FindFreeSlot()
{
    for (uint32 i = 0; i < NumInstances; ++i)
    {
        uint32 index = FreeSlotIndexHint++ % NumInstances;
        ArchetypeInstance* instance = GetEntityPtr(index);
        if (instance->EntityId == EntityId::Invalid)
        {
            return index;
        }
    }
    return Index<uint32>::None;
}
