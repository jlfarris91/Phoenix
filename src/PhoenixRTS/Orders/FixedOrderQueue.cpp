#include "FixedOrderQueue.h"

Phoenix::uint32 Phoenix::RTS::FixedOrderQueue::GetCapacity() const
{
    return Storage.GetCapacity();
}

Phoenix::uint32 Phoenix::RTS::FixedOrderQueue::GetAllocSizeBytes(uint32 capacity)
{
    return TStorage::GetAllocSizeBytes(capacity);
}

Phoenix::uint32 Phoenix::RTS::FixedOrderQueue::GetAllocSizeBytes() const
{
    return Storage.GetAllocSizeBytes();
}

Phoenix::uint32 Phoenix::RTS::FixedOrderQueue::GetNum() const
{
    return Storage.GetNum();
}

Phoenix::uint32 Phoenix::RTS::FixedOrderQueue::GetNumValidOrders() const
{
    return Storage.GetNumValidItems();
}

bool Phoenix::RTS::FixedOrderQueue::IsEmpty() const
{
    return Storage.IsEmpty();
}

bool Phoenix::RTS::FixedOrderQueue::IsFull() const
{
    return Storage.IsFull();
}

bool Phoenix::RTS::FixedOrderQueue::ContainsOrder(ECS::EntityId entity, const Order& order) const
{
    return Storage.Contains({ entity, order });
}

bool Phoenix::RTS::FixedOrderQueue::EnqueueOrder(ECS::EntityId entity, const Order& order)
{
    return Storage.EmplaceBack(entity, order);
}

bool Phoenix::RTS::FixedOrderQueue::DequeueOrder(ECS::EntityId entity, Order& outOrder)
{
    EntityOrder item;
    if (!Storage.RemoveFirstItemByKey(entity, item))
    {
        return false;
    }

    outOrder = item.Order;
    return true;
}

bool Phoenix::RTS::FixedOrderQueue::InsertOrder(ECS::EntityId entity, const Order& order, uint32 orderIndex)
{
    return Storage.InsertItem({ entity, order }, orderIndex);
}

bool Phoenix::RTS::FixedOrderQueue::RemoveOrder(ECS::EntityId entity, const Order& order)
{
    return Storage.Remove({ entity, order });
}

bool Phoenix::RTS::FixedOrderQueue::RemoveOrder(ECS::EntityId entity, uint32 orderIndex)
{
    return Storage.RemoveItemByKey(entity, orderIndex);
}

Phoenix::uint32 Phoenix::RTS::FixedOrderQueue::RemoveAllOrders(ECS::EntityId entity)
{
    return Storage.RemoveAll(entity);
}

const Phoenix::RTS::Order* Phoenix::RTS::FixedOrderQueue::GetFirstOrder(ECS::EntityId entity) const
{
    uint32 outIndex;
    const EntityOrder* item = Storage.GetFirstItem(entity, outIndex);
    return item ? &item->Order : nullptr;
}

const Phoenix::RTS::Order* Phoenix::RTS::FixedOrderQueue::GetFirstOrder(ECS::EntityId entity, uint32& outIndex) const
{
    const EntityOrder* item = Storage.GetFirstItem(entity, outIndex);
    return item ? &item->Order : nullptr;
}

const Phoenix::RTS::Order* Phoenix::RTS::FixedOrderQueue::GetNextOrder(ECS::EntityId entity, uint32 currIndex,
    uint32& outIndex) const
{
    const EntityOrder* item = Storage.GetNextItem(entity, currIndex, outIndex);
    return item ? &item->Order : nullptr;
}

const Phoenix::RTS::Order* Phoenix::RTS::FixedOrderQueue::GetOrder(ECS::EntityId entity, uint32 orderIndex) const
{
    const EntityOrder* item = Storage.GetItem(entity, orderIndex);
    return item ? &item->Order : nullptr;
}

Phoenix::uint32 Phoenix::RTS::FixedOrderQueue::GetNumOrders(ECS::EntityId entity) const
{
    return Storage.GetNumItems(entity);
}

void Phoenix::RTS::FixedOrderQueue::Sort()
{
    Storage.Sort();
}
