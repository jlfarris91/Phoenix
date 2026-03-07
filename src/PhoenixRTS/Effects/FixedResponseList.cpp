#include "FixedResponseList.h"

Phoenix::uint32 Phoenix::RTS::FixedResponseList::GetCapacity() const
{
    return Storage.GetCapacity();
}

Phoenix::uint32 Phoenix::RTS::FixedResponseList::GetAllocSizeBytes(uint32 capacity)
{
    return TStorage::GetAllocSizeBytes(capacity);
}

Phoenix::uint32 Phoenix::RTS::FixedResponseList::GetAllocSizeBytes() const
{
    return Storage.GetAllocSizeBytes();
}

Phoenix::uint32 Phoenix::RTS::FixedResponseList::GetNum() const
{
    return Storage.GetNum();
}

Phoenix::uint32 Phoenix::RTS::FixedResponseList::GetNumValidResponses() const
{
    return Storage.GetNumValidItems();
}

bool Phoenix::RTS::FixedResponseList::ContainsResponse(ECS::EntityId entity, const FName& response) const
{
    return Storage.Contains({ entity, response });
}

bool Phoenix::RTS::FixedResponseList::AddResponse(ECS::EntityId entity, const FName& response)
{
    return Storage.PushBackUnique({ entity, response });
}

bool Phoenix::RTS::FixedResponseList::RemoveResponse(ECS::EntityId entity, const FName& response)
{
    return Storage.Remove({ entity, response });
}

Phoenix::uint32 Phoenix::RTS::FixedResponseList::ClearResponses(ECS::EntityId entity)
{
    return Storage.RemoveAll(entity);
}

Phoenix::FName Phoenix::RTS::FixedResponseList::GetFirstResponse(ECS::EntityId entity, uint32& outIndex) const
{
    const EntityResponse* item = Storage.GetFirstItem(entity, outIndex);
    return item ? item->Response : FName::None;
}

Phoenix::FName Phoenix::RTS::FixedResponseList::GetNextResponse(ECS::EntityId entity, uint32 currIndex,
    uint32& outIndex) const
{
    const EntityResponse* item = Storage.GetNextItem(entity, currIndex, outIndex);
    return item ? item->Response : FName::None;
}

void Phoenix::RTS::FixedResponseList::Sort()
{
    Storage.Sort();
}
