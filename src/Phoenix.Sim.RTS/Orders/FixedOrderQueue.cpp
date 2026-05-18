#include "FixedOrderQueue.h"

using namespace Phoenix;
using namespace Phoenix::RTS;

void FixedOrderQueue::Construct(BlockBufferAllocator& allocator, const Config& config)
{
    Storage.Construct(allocator, config.Capacity);
}

BlockBufferLayout FixedOrderQueue::StaticLayout(const Config& config)
{
    return BlockBufferLayout::For<FixedOrderQueue>().Container<TStorage>(config.Capacity);
}

uint32 FixedOrderQueue::GetCapacity() const
{
    return Storage.GetCapacity();
}

uint32 FixedOrderQueue::GetNum() const
{
    return Storage.GetNum();
}

uint32 FixedOrderQueue::GetNumValidOrders() const
{
    return Storage.GetNumValidItems();
}

bool FixedOrderQueue::IsEmpty() const
{
    return Storage.IsEmpty();
}

bool FixedOrderQueue::IsFull() const
{
    return Storage.IsFull();
}

bool FixedOrderQueue::ContainsOrder(ECS::EntityId entity, const Order& order) const
{
    return Storage.Contains({ entity, order });
}

bool FixedOrderQueue::EnqueueOrder(ECS::EntityId entity, const Order& order)
{
    return Storage.EmplaceBack(entity, order);
}

bool FixedOrderQueue::DequeueOrder(ECS::EntityId entity, Order& outOrder)
{
    EntityOrder item;
    if (!Storage.RemoveFirstItemByKey(entity, item))
    {
        return false;
    }

    outOrder = item.Order;
    return true;
}

bool FixedOrderQueue::InsertOrder(ECS::EntityId entity, const Order& order, uint32 orderIndex)
{
    return Storage.InsertItem({ entity, order }, orderIndex);
}

bool FixedOrderQueue::RemoveOrder(ECS::EntityId entity, const Order& order)
{
    return Storage.Remove({ entity, order });
}

bool FixedOrderQueue::RemoveOrder(ECS::EntityId entity, uint32 orderIndex)
{
    return Storage.RemoveItemByKey(entity, orderIndex);
}

uint32 FixedOrderQueue::RemoveAllOrders(ECS::EntityId entity)
{
    return Storage.RemoveAll(entity);
}

const Order* FixedOrderQueue::GetFirstOrder(ECS::EntityId entity) const
{
    uint32 outIndex;
    const EntityOrder* item = Storage.GetFirstItem(entity, outIndex);
    return item ? &item->Order : nullptr;
}

const Order* FixedOrderQueue::GetFirstOrder(ECS::EntityId entity, uint32& outIndex) const
{
    const EntityOrder* item = Storage.GetFirstItem(entity, outIndex);
    return item ? &item->Order : nullptr;
}

const Order* FixedOrderQueue::GetNextOrder(ECS::EntityId entity, uint32 currIndex,
    uint32& outIndex) const
{
    const EntityOrder* item = Storage.GetNextItem(entity, currIndex, outIndex);
    return item ? &item->Order : nullptr;
}

const Order* FixedOrderQueue::GetOrder(ECS::EntityId entity, uint32 orderIndex) const
{
    const EntityOrder* item = Storage.GetItem(entity, orderIndex);
    return item ? &item->Order : nullptr;
}

uint32 FixedOrderQueue::GetNumOrders(ECS::EntityId entity) const
{
    return Storage.GetNumItems(entity);
}

void FixedOrderQueue::Sort()
{
    Storage.Sort();
}
