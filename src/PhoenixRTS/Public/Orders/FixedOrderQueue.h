
#pragma once

#include "Units/UnitId.h"
#include "Orders.h"
#include "Containers/FixedSortedList.h"

namespace Phoenix::RTS
{
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

        bool ContainsOrder(UnitId entity, const Order& order) const
        {
            return Items.Contains({ entity, order });
        }

        bool EnqueueOrder(UnitId entity, const Order& order)
        {
            return Items.EmplaceBack(entity, order);
        }

        bool DequeueOrder(UnitId entity, Order& outOrder)
        {
            EntityOrder item;
            if (!Items.RemoveSubItemAndReturn(entity, 0, item))
            {
                return false;
            }

            outOrder = item.Order;
            return true;
        }

        bool InsertOrder(UnitId entity, const Order& order, uint32 orderIndex)
        {
            return Items.InsertSubItem({ entity, order }, orderIndex);
        }

        bool RemoveOrder(UnitId entity, const Order& order)
        {
            return Items.Remove({ entity, order });
        }

        bool RemoveOrder(UnitId entity, uint32 orderIndex)
        {
            return Items.RemoveSubItem(entity, orderIndex);
        }

        uint32 RemoveAllOrders(UnitId entity)
        {
            return Items.RemoveAll(entity);
        }

        const Order* GetFirstOrder(UnitId entity, uint32& outIndex) const
        {
            const EntityOrder* item = Items.GetFirstSubItem(entity, outIndex);
            return item ? &item->Order : nullptr;
        }

        const Order* GetNextOrder(UnitId entity, uint32 currIndex, uint32& outIndex) const
        {
            const EntityOrder* item = Items.GetNextSubItem(entity, currIndex, outIndex);
            return item ? &item->Order : nullptr;
        }

        const Order* GetOrder(UnitId entity, uint32 orderIndex) const
        {
            const EntityOrder* item = Items.GetSubItem(entity, orderIndex);
            return item ? &item->Order : nullptr;
        }

        uint32 GetNumOrders(UnitId entity) const
        {
            return Items.GetNumSubItems(entity);
        }

        template <class TCallback>
        void ForEachOrder(UnitId entity, const TCallback& callback) const
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

        struct EntityOrder
        {
            EntityOrder() = default;
            EntityOrder(UnitId entity, const Order& order = {})
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

            UnitId Entity;
            Order Order;
        };

        struct GetItemKey
        {
            UnitId operator()(const EntityOrder& item) const
            {
                return item.Entity;
            }
        };

        TFixedSortedList<EntityOrder, Capacity, GetItemKey> Items;
    };
}
