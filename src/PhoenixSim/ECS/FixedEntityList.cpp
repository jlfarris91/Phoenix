#include "FixedEntityList.h"

using namespace Phoenix;
using namespace Phoenix::ECS;

void FixedEntityList::Construct(BlockBufferAllocator& allocator, const Config& config)
{
    Storage.Construct(allocator, config.Capacity);
}

BlockBufferLayout FixedEntityList::StaticLayout(const Config& config)
{
    return BlockBufferLayout::For<FixedEntityList>().Container<TStorage>(config.Capacity);
}

uint32 FixedEntityList::GetCapacity() const
{
    return Storage.GetCapacity();
}

const Entity* FixedEntityList::GetData() const
{
    return Storage.GetData();
}

uint32 FixedEntityList::GetNumActive() const
{
    return NumActiveEntities;
}

uint32 FixedEntityList::GetNumHighWaterMark() const
{
    return NumHighWaterMark;
}

bool FixedEntityList::IsValid(EntityId entityId) const
{
    return GetEntityPtr(entityId) != nullptr;
}

uint32 FixedEntityList::GetEntityIndex(EntityId entityId) const
{
    return static_cast<uint32>(entityId) % GetCapacity();
}

Entity* FixedEntityList::GetEntityPtr(EntityId entityId)
{
    uint32 index = GetEntityIndex(entityId);
    if (!Storage.IsValidIndex(index))
    {
        return nullptr;
    }
    Entity& entity = Storage[index];
    return entity.Id == entityId ? &entity : nullptr;
}

const Entity* FixedEntityList::GetEntityPtr(EntityId entityId) const
{
    uint32 index = GetEntityIndex(entityId);
    if (!Storage.IsValidIndex(index))
    {
        return nullptr;
    }
    const Entity& entity = Storage[index];
    return entity.Id == entityId ? &entity : nullptr;
}

Entity& FixedEntityList::GetEntityRef(EntityId entityId)
{
    uint32 index = GetEntityIndex(entityId);
    return Storage[index];
}

const Entity& FixedEntityList::GetEntityRef(EntityId entityId) const
{
    uint32 index = GetEntityIndex(entityId);
    return Storage[index];
}

EntityId FixedEntityList::Acquire(const FName& kind)
{
    if (NumActiveEntities + 1 == GetCapacity())
    {
        return EntityId::Invalid;
    }

    // Find the first invalid entity index
    uint32 entityIdx = 1;
    for (; entityIdx < GetCapacity(); ++entityIdx)
    {
        if (!Storage.IsValidIndex(entityIdx))
        {
            break;
        }

        Entity& entity = Storage[entityIdx];
        entityid_t currId = entity.Id;
        uint32 currIdx = GetEntityIndex(currId);

        if (currIdx != entityIdx && currIdx == 0)
        {
            break;
        }
    }

    if (entityIdx == GetCapacity())
    {
        return EntityId::Invalid;
    }

    if (!Storage.IsValidIndex(entityIdx))
    {
        Storage.SetNum(entityIdx + 1);
        NumHighWaterMark = Storage.GetNum();
    }

    Entity& entity = Storage[entityIdx];
    entity.Kind = kind;

    entityid_t currId = entity.Id;
    if (currId == Index<uint32>::None)
    {
        currId = 0;
    }

    entity.Id = currId + GetCapacity() + entityIdx;

#if DEBUG
    //PHX_ASSERT(IsValid(entityId));
    if (!IsValid(entity.Id))
    {
        PHX_DEBUG_BREAK();
    }
#endif

    ++NumActiveEntities;

    return entity.Id;
}

bool FixedEntityList::Release(EntityId entityId)
{
    uint32 currIdx = GetEntityIndex(entityId);
    PHX_ASSERT(Storage.IsValidIndex(currIdx));

    // Entity is inactive
    if (currIdx == 0)
    {
        return false;
    }

    Entity& entity = Storage[currIdx];

    // Entity has already been marked for death
    if (entity.Id != entityId)
    {
        return false;
    }

    // Make a new entity id where the index doesn't match and isn't zero.
    entity.Id = (static_cast<entityid_t>(entity.Id) ^ 1) | 2;

#if DEBUG
    // PHX_ASSERT(!IsMarkedForDeath(entity.Id, currIdx));
    if (!IsMarkedForDeath(entity.Id, currIdx))
    {
        PHX_DEBUG_BREAK();
    }
#endif

    return true;
}

uint32 FixedEntityList::ReclaimEntities(const FOnEntityReclaimed& callback)
{
    uint32 numReclaimed = 0;
    uint32 numEntities = Storage.GetNum();
    uint32 capacity = Storage.GetCapacity();
    for (uint32 i = 0; i < numEntities; ++i)
    {
        Entity& entity = Storage[i];

        if (!IsMarkedForDeath(entity.Id, i))
        {
            continue;
        }

        // Rebuild the entity id prior to being marked for death
        EntityId prevEntityId = uint32(entity.Id / capacity) * capacity + i;

        callback(prevEntityId);
        ReclaimEntity(entity);

        ++numReclaimed;
    }
    return numReclaimed;
}

bool FixedEntityList::IsMarkedForDeath(EntityId entityId, uint32 index) const
{
    uint32 currentIdx = entityId % GetCapacity();

    // Entity is active
    if (currentIdx == index)
    {
        return false;
    }

    // Entity is inactive
    if (currentIdx == 0)
    {
        return false;
    }

    return true;
}

void FixedEntityList::ReclaimEntity(Entity& entity)
{
    uint32 currIdx = GetEntityIndex(entity.Id);

    PHX_ASSERT(currIdx != 0);

    // Reset the index to 0 to represent a free entity
    entity.Id = static_cast<entityid_t>(entity.Id) - currIdx;

    --NumActiveEntities;
}
