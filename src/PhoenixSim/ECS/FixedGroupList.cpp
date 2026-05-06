#include "FixedGroupList.h"

using namespace Phoenix;
using namespace Phoenix::ECS;

GroupEntity::GroupEntity(EntityId group, EntityId entity)
    : Group(group)
    , Entity(entity)
{
}

bool GroupEntity::operator==(const GroupEntity& other) const
{
    return Group == other.Group && Entity == other.Entity;
}

bool GroupEntity::IsValid() const
{
    return Entity != EntityId::Invalid;
}

void GroupEntity::Invalidate()
{
    Entity = EntityId::Invalid;
}

EntityId GroupEntity::GetItemKey::operator()(const GroupEntity& item) const
{
    return item.Group;
}

void FixedGroupList::Construct(BlockBufferAllocator& allocator, const Config& config)
{
    Storage.Construct(allocator, config.Capacity);
}

BlockBufferLayout FixedGroupList::StaticLayout(const Config& config)
{
    return BlockBufferLayout::For<FixedGroupList>().Container<TStorage>(config.Capacity);
}

uint32 FixedGroupList::GetCapacity() const
{
    return Storage.GetCapacity();
}

const GroupEntity* FixedGroupList::GetData() const
{
    return Storage.GetData();
}

void FixedGroupList::Sort()
{
    Storage.Sort();
}

uint32 FixedGroupList::GetNumValidPairs() const
{
    return Storage.GetNumValidItems();
}

bool FixedGroupList::ContainsEntity(EntityId group, EntityId entity) const
{
    return Storage.Contains({ group, entity });
}

bool FixedGroupList::AddEntity(EntityId group, EntityId entity)
{
    return Storage.PushBackUnique({ group, entity });
}

bool FixedGroupList::RemoveEntity(EntityId group, EntityId entity)
{
    return Storage.Remove({ group, entity });
}

bool FixedGroupList::RemoveEntityFromAllGroups(EntityId entity)
{
    return Storage.RemoveAll(RemoveEntityFromAllGroupsPred { entity });
}

uint32 FixedGroupList::ClearGroup(EntityId group)
{
    return Storage.RemoveAll(group);
}

uint32 FixedGroupList::GetNumEntities(EntityId group) const
{
    return Storage.GetNumItems(group);
}

EntityId FixedGroupList::GetFirstEntity(EntityId group, uint32& outIndex) const
{
    const GroupEntity* item = Storage.GetFirstItem(group, outIndex);
    return item ? item->Entity : EntityId::Invalid;
}

EntityId FixedGroupList::GetNextEntity(EntityId group, uint32 currIndex, uint32& outIndex) const
{
    const GroupEntity* item = Storage.GetNextItem(group, currIndex, outIndex);
    return item ? item->Entity : EntityId::Invalid;
}

EntityId FixedGroupList::RemoveEntityFromAllGroupsPred::operator()(const GroupEntity& item) const
{
    return item.Entity == Entity;
}