#include "FixedTagList.h"

using namespace Phoenix;
using namespace Phoenix::ECS;

EntityTag::EntityTag(EntityId entity, FName tag)
    : Entity(entity)
    , Tag(tag)
{
}

bool EntityTag::operator==(const EntityTag& other) const
{
    return Entity == other.Entity && Tag == other.Tag;
}

bool EntityTag::IsValid() const
{
    return Tag != FName::None;
}

void EntityTag::Invalidate()
{
    Tag = FName::None;
}

EntityId EntityTag::GetItemKey::operator()(const EntityTag& item) const
{
    return item.Entity;
}

uint32 FixedTagList::GetCapacity() const
{
    return Storage.GetCapacity();
}

uint32 FixedTagList::GetAllocSizeBytes(uint32 capacity)
{
    return TStorage::GetAllocSizeBytes(capacity);
}

uint32 FixedTagList::GetAllocSizeBytes() const
{
    return Storage.GetAllocSizeBytes();
}

const EntityTag* FixedTagList::GetData() const
{
    return Storage.GetData();
}

void FixedTagList::Sort()
{
    Storage.Sort();
}

uint32 FixedTagList::GetNumValidTags() const
{
    return Storage.GetNumValidItems();
}

bool FixedTagList::HasTag(EntityId entity, const FName& tag) const
{
    return Storage.Contains({ entity, tag });
}

bool FixedTagList::AddTag(EntityId entity, const FName& tag)
{
    return Storage.PushBackUnique({ entity, tag });
}

bool FixedTagList::RemoveTag(EntityId entity, const FName& tag)
{
    return Storage.Remove({ entity, tag });
}

uint32 FixedTagList::RemoveAllTags(EntityId entity)
{
    return Storage.RemoveAll(entity);
}

FName FixedTagList::GetFirstTag(EntityId entity, uint32& outIndex) const
{
    const EntityTag* item = Storage.GetFirstItem(entity, outIndex);
    return item ? item->Tag : FName::None;
}

FName FixedTagList::GetNextTag(EntityId entity, uint32 currIndex, uint32& outIndex) const
{
    const EntityTag* item = Storage.GetNextItem(entity, currIndex, outIndex);
    return item ? item->Tag : FName::None;
}
