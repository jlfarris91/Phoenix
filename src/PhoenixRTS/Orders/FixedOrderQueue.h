
#pragma once

#include "PhoenixSim/ECS/EntityId.h"
#include "PhoenixSim/Containers/FixedSortedList.h"

#include "PhoenixRTS/Orders/Orders.h"

namespace Phoenix::RTS
{
    struct PHOENIX_RTS_API EntityOrder
    {
        EntityOrder() = default;
        EntityOrder(ECS::EntityId entity, const Order& order = {})
            : Entity(entity)
            , Order(order)
        {
        }

        bool operator==(const EntityOrder& other) const
        {
            return Entity == other.Entity && Order == other.Order;
        }

        bool IsValid() const { return Order != RTS::Order(); }
        void Invalidate() { Order = RTS::Order(); }

        ECS::EntityId Entity;
        Order Order;

        struct GetItemKey
        {
            ECS::EntityId operator()(const EntityOrder& item) const
            {
                return item.Entity;
            }
        };
    };

    class FixedOrderQueue
    {
        using TStorage = TFixedSortedList<EntityOrder, EntityOrder::GetItemKey>;

    public:

        FixedOrderQueue() = default;

        template <class TAllocator>
        FixedOrderQueue(TAllocator& allocator, uint32 capacity)
            : Storage(allocator, capacity)
        {
        }

        template <class TAllocator>
        FixedOrderQueue(TAllocator& allocator, uint32 capacity, const FixedOrderQueue& other)
            : Storage(allocator, capacity, other.Storage)
        {
        }

        uint32 GetCapacity() const;

        static uint32 GetAllocSizeBytes(uint32 capacity);

        uint32 GetAllocSizeBytes() const;

        uint32 GetNum() const;

        uint32 GetNumValidOrders() const;

        bool IsEmpty() const;

        bool IsFull() const;

        bool ContainsOrder(ECS::EntityId entity, const Order& order) const;

        bool EnqueueOrder(ECS::EntityId entity, const Order& order);

        bool DequeueOrder(ECS::EntityId entity, Order& outOrder);

        bool InsertOrder(ECS::EntityId entity, const Order& order, uint32 orderIndex);

        bool RemoveOrder(ECS::EntityId entity, const Order& order);

        bool RemoveOrder(ECS::EntityId entity, uint32 orderIndex);

        uint32 RemoveAllOrders(ECS::EntityId entity);

        const Order* GetFirstOrder(ECS::EntityId entity) const;

        const Order* GetFirstOrder(ECS::EntityId entity, uint32& outIndex) const;

        const Order* GetNextOrder(ECS::EntityId entity, uint32 currIndex, uint32& outIndex) const;

        const Order* GetOrder(ECS::EntityId entity, uint32 orderIndex) const;

        uint32 GetNumOrders(ECS::EntityId entity) const;

        template <class TCallback>
        void ForEach(const TCallback& callback) const
        {
            Storage.ForEachItem(callback);
        }

        template <class TCallback>
        void ForEachOrder(ECS::EntityId entity, const TCallback& callback) const
        {
            Storage.ForEachItemProjected(entity, [&](const EntityOrder& item) -> const Order&
            {
                return item.Order;
            }, callback);
        }

        void Sort();

    private:

        TStorage Storage;
    };
}
