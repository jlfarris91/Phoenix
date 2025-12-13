
#pragma once

#include "EntityId.h"
#include "Orders.h"
#include "Containers/FixedSortedList.h"

namespace Phoenix::RTS
{
    struct EntityOrder
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
    };

    template <size_t N>
    class FixedOrderQueue
    {
    public:

        static constexpr uint32 Capacity = N;

        uint32 GetSize() const
        {
            return Items.GetSize();
        }

        uint32 GetNumValidOrders() const
        {
            return Items.GetNumValidItems();
        }

        bool ContainsOrder(ECS::EntityId entity, const Order& order) const
        {
            return Items.Contains({ entity, order });
        }

        bool EnqueueOrder(ECS::EntityId entity, const Order& order)
        {
            return Items.EmplaceBack(entity, order);
        }

        bool DequeueOrder(ECS::EntityId entity, Order& outOrder)
        {
            EntityOrder item;
            if (!Items.RemoveSubItemAndReturn(entity, 0, item))
            {
                return false;
            }

            outOrder = item.Order;
            return true;
        }

        bool InsertOrder(ECS::EntityId entity, const Order& order, uint32 orderIndex)
        {
            return Items.InsertSubItem({ entity, order }, orderIndex);
        }

        bool RemoveOrder(ECS::EntityId entity, const Order& order)
        {
            return Items.Remove({ entity, order });
        }

        bool RemoveOrder(ECS::EntityId entity, uint32 orderIndex)
        {
            return Items.RemoveSubItem(entity, orderIndex);
        }

        uint32 RemoveAllOrders(ECS::EntityId entity)
        {
            return Items.RemoveAll(entity);
        }

        const Order* GetFirstOrder(ECS::EntityId entity, uint32& outIndex) const
        {
            const EntityOrder* item = Items.GetFirstSubItem(entity, outIndex);
            return item ? &item->Order : nullptr;
        }

        const Order* GetNextOrder(ECS::EntityId entity, uint32 currIndex, uint32& outIndex) const
        {
            const EntityOrder* item = Items.GetNextSubItem(entity, currIndex, outIndex);
            return item ? &item->Order : nullptr;
        }

        const Order* GetOrder(ECS::EntityId entity, uint32 orderIndex) const
        {
            const EntityOrder* item = Items.GetSubItem(entity, orderIndex);
            return item ? &item->Order : nullptr;
        }

        uint32 GetNumOrders(ECS::EntityId entity) const
        {
            return Items.GetNumSubItems(entity);
        }

        template <class TCallback>
        void ForEach(const TCallback& callback) const
        {
            Items.ForEachItem( [&](const EntityOrder& item)
            {
                callback(item);
            });
        }

        template <class TCallback>
        void ForEachOrder(ECS::EntityId entity, const TCallback& callback) const
        {
            Items.ForEachSubItem(entity, [&](const EntityOrder& item)
            {
                callback(item.Order);
            });
        }

        void Sort()
        {
            Items.Sort();
        }

    private:

        struct GetItemKey
        {
            ECS::EntityId operator()(const EntityOrder& item) const
            {
                return item.Entity;
            }
        };

        TFixedSortedList<EntityOrder, Capacity, GetItemKey> Items;
    };
}
