#include "FixedResponseList.h"

using namespace Phoenix;
using namespace Phoenix::RTS;

void FixedResponseList::Construct(BlockBufferAllocator& allocator, const Config& config)
{
    Storage.Construct(allocator, config.Capacity);
}

BlockBufferLayout FixedResponseList::StaticLayout(const Config& config)
{
    return BlockBufferLayout::For<FixedResponseList>().Container<TStorage>(config.Capacity);
}

uint32 FixedResponseList::GetCapacity() const
{
    return Storage.GetCapacity();
}

uint32 FixedResponseList::GetNum() const
{
    return Storage.GetNum();
}

uint32 FixedResponseList::GetNumValidResponses() const
{
    return Storage.GetNumValidItems();
}

bool FixedResponseList::ContainsResponse(ECS::EntityId entity, const FName& response) const
{
    return Storage.Contains({ entity, response });
}

bool FixedResponseList::AddResponse(ECS::EntityId entity, const FName& response)
{
    return Storage.PushBackUnique({ entity, response });
}

bool FixedResponseList::RemoveResponse(ECS::EntityId entity, const FName& response)
{
    return Storage.Remove({ entity, response });
}

uint32 FixedResponseList::ClearResponses(ECS::EntityId entity)
{
    return Storage.RemoveAll(entity);
}

FName FixedResponseList::GetFirstResponse(ECS::EntityId entity, uint32& outIndex) const
{
    const EntityResponse* item = Storage.GetFirstItem(entity, outIndex);
    return item ? item->Response : FName::None;
}

FName FixedResponseList::GetNextResponse(ECS::EntityId entity, uint32 currIndex,
    uint32& outIndex) const
{
    const EntityResponse* item = Storage.GetNextItem(entity, currIndex, outIndex);
    return item ? item->Response : FName::None;
}

void FixedResponseList::Sort()
{
    Storage.Sort();
}
