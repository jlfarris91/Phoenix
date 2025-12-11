
#pragma once

#include <algorithm>

#include "EntityId.h"
#include "Orders.h"
#include "Containers/FixedArray.h"

namespace Phoenix::RTS
{
    template <uint32 N>
    class TFixedOrderQueue
    {
    public:

        static constexpr uint32 Capacity = N;

        struct EntityOrder
        {
            ECS::EntityId Id;
            Order Order;
            constexpr bool IsValid() const { return Order != RTS::Order(); }
            void Invalidate() { Order = RTS::Order(); }
        };

        constexpr uint32 GetSize() const
        {
            return Orders.Num();
        }

        constexpr uint32 GetNumValidOrders() const
        {
            return NumValidOrders;
        }

        bool EnqueueOrder(const ECS::EntityId& entityId, const Order& order)
        {
            if (Orders.IsFull())
            {
                return false;
            }

            Orders.Add({ entityId, order });
            ++NumValidOrders;
            return true;
        }

        bool DequeueOrder(const ECS::EntityId& entityId, Order& outOrder)
        {
            return RemoveAndReturnOrder(entityId, 0, outOrder);
        }

        bool InsertOrder(const ECS::EntityId& entityId, const Order& order, uint32 orderIndex)
        {
            if (Orders.IsFull())
            {
                return false;
            }

            EntityOrder* entityOrder = GetEntityOrder(entityId, orderIndex);
            if (!entityOrder)
            {
                // Couldn't find an entity order at the order index so just enqueue.
                return EnqueueOrder(entityId, order);
            }

            uint32 index = static_cast<uint32>(entityOrder - &Orders[0]);
            InsertOrderAtIndex(entityId, order, index);

            // Order is being inserted into the sorted section so increase the number of sorted orders.
            if (index < SortedNum)
            {
                ++SortedNum;
            }

            return true;
        }

        bool RemoveOrder(const ECS::EntityId& entityId, uint32 orderIndex)
        {
            if (Orders.IsEmpty())
            {
                return false;
            }

            EntityOrder* entityOrder = GetEntityOrder(entityId, orderIndex);
            if (!entityOrder)
            {
                return false;
            }

            entityOrder->Invalidate();
            --NumValidOrders;

            return true;
        }

        bool RemoveAndReturnOrder(const ECS::EntityId& entityId, uint32 orderIndex, Order& outOrder)
        {
            if (Orders.IsEmpty())
            {
                outOrder = Order{};
                return false;
            }

            EntityOrder* entityOrder = GetEntityOrder(entityId, orderIndex);
            if (!entityOrder)
            {
                outOrder = Order{};
                return false;
            }

            outOrder = entityOrder->Order;

            entityOrder->Invalidate();
            --NumValidOrders;

            return true;
        }

        uint32 RemoveAllOrders(const ECS::EntityId& entityId)
        {
            uint32 numRemoved = 0;

            auto begin = Orders.begin();
            auto end = Orders.end();
            auto sortedEnd = begin + SortedNum;

            // Search the sorted section
            if (sortedEnd != begin)
            {
                auto iter = std::lower_bound(begin, sortedEnd, EntityOrder{ entityId, Order{} }, SortOrderById());
                while (iter != sortedEnd && iter->Id == entityId)
                {
                    if (iter->IsValid())
                    {
                        iter->Invalidate();
                        ++numRemoved;
                        --NumValidOrders;
                    }
                    ++iter;
                }
            }

            // Search the unsorted section
            auto iter = sortedEnd;
            while (iter != end)
            {
                if (iter->Id == entityId && iter->IsValid())
                {
                    iter->Invalidate();
                    ++numRemoved;
                    --NumValidOrders;
                }
                ++iter;
            }

            return numRemoved;
        }

        const Order* GetHeadOrder(const ECS::EntityId& entityId, uint32& outIndex) const
        {
            auto begin = Orders.begin();
            auto end = Orders.end();
            auto sortedEnd = begin + SortedNum;

            // Search the sorted section
            if (sortedEnd != begin)
            {
                auto iter = std::lower_bound(begin, sortedEnd, EntityOrder{ entityId, Order{} }, SortOrderById());
                while (iter != sortedEnd && iter->Id == entityId)
                {
                    if (iter->IsValid())
                    {
                        outIndex = static_cast<uint32>(iter - begin);
                        return &iter->Order;
                    }
                    ++iter;
                }
            }

            // Search the unsorted section
            auto iter = sortedEnd;
            while (iter != end)
            {
                if (iter->Id == entityId && iter->IsValid())
                {
                    outIndex = static_cast<uint32>(iter - begin);
                    return &iter->Order;
                }
                ++iter;
            }

            return nullptr;
        }

        const Order* GetNextOrder(const ECS::EntityId& entityId, uint32 currIndex, uint32& outIndex) const
        {
            uint32 index = currIndex + 1;

            // Search the sorted section
            while (index < SortedNum && Orders[index].Id == entityId)
            {
                const EntityOrder& order = Orders[index];
                if (order.IsValid())
                {
                    outIndex = index;
                    return &order.Order;
                }
                ++index;
            }

            // Search the unsorted section
            index = SortedNum;
            while (index < Orders.Num())
            {
                const EntityOrder& order = Orders[index];
                if (order.Id == entityId && order.IsValid())
                {
                    outIndex = index;
                    return &order.Order;
                }
                ++index;
            }

            outIndex = Index<uint32>::None;
            return nullptr;
        }

        const Order* GetOrder(const ECS::EntityId& entityId, uint32 orderIndex) const
        {
            const EntityOrder* entityOrder = GetEntityOrder(entityId, orderIndex);
            return entityOrder ? &entityOrder->Order : nullptr;
        }

        uint32 GetNumOrders(const ECS::EntityId& entityId) const
        {
            auto begin = Orders.begin();
            auto end = Orders.end();
            auto sortedEnd = begin + SortedNum;
            uint32 numOrders = 0;

            // Search the sorted section
            if (sortedEnd != begin)
            {
                auto iter = std::lower_bound(begin, sortedEnd, EntityOrder{ entityId, Order{} }, SortOrderById());
                while (iter != sortedEnd && iter->Id == entityId)
                {
                    if (iter->IsValid())
                    {
                        ++numOrders;
                    }
                }
            }

            // Search the unsorted section
            auto iter = sortedEnd;
            while (iter != end)
            {
                if (iter->Id == entityId && iter->IsValid())
                {
                    ++numOrders;
                }
                ++iter;
            }

            return numOrders;
        }

        template <class TCallback>
        void ForEachOrder(const ECS::EntityId& entityId, const TCallback& callback) const
        {
            auto begin = Orders.begin();
            auto end = Orders.end();
            auto sortedEnd = begin + SortedNum;

            // Search the sorted section
            if (sortedEnd != begin)
            {
                auto iter = std::lower_bound(begin, sortedEnd, { entityId, Order{} }, SortOrderById());
                while (iter != sortedEnd && iter->Id == entityId)
                {
                    if (iter->IsValid())
                    {
                        callback(*iter);
                    }
                }
            }

            // Search the unsorted section
            auto iter = sortedEnd;
            while (iter != end)
            {
                if (iter->Id == entityId && iter->IsValid())
                {
                    callback(*iter);
                }
                ++iter;
            }
        }

        void Sort()
        {
            std::stable_sort(Orders.begin(), Orders.end(), SortInvalidOrdersToBack());

            auto iter = Orders.end();
            while (iter != Orders.begin() && !iter->IsValid())
            {
                --iter;
            }

            SortedNum = static_cast<uint32>(iter - Orders.begin());
            Orders.SetNum(SortedNum);
        }

    private:

        struct SortOrderById
        {
            constexpr bool operator()(const EntityOrder& a, const EntityOrder& b) const
            {
                return a.Id < b.Id;
            }
        };

        struct SortInvalidOrdersToBack
        {
            constexpr bool operator()(const EntityOrder& a, const EntityOrder& b) const
            {
                // Sort invalid records to the back.
                if (!a.IsValid())
                {
                    return false;
                }
                if (!b.IsValid())
                {
                    return true;
                }
                return a.Id < b.Id;
            }
        };

        struct InvalidOrderPred
        {
            constexpr bool operator()(const EntityOrder& order) const
            {
                return !order.IsValid();
            }
        };

        uint32 FindIndexOfEntityOrder(const ECS::EntityId& entityId, uint32 orderIndex) const
        {
            auto begin = Orders.begin();
            auto end = Orders.end();
            auto sortedEnd = begin + SortedNum;

            // Search the sorted section
            if (sortedEnd != begin)
            {
                auto iter = std::lower_bound(begin, sortedEnd, EntityOrder{ entityId, Order{} }, SortOrderById());
                while (iter != sortedEnd && iter->Id == entityId)
                {
                    if (iter->IsValid() && orderIndex-- == 0)
                    {
                        return static_cast<uint32>(iter - begin);
                    }
                    ++iter;
                }
            }

            // Search the unsorted section
            auto iter = sortedEnd;
            while (iter != end)
            {
                if (iter->Id == entityId && iter->IsValid() && orderIndex-- == 0)
                {
                    return static_cast<uint32>(iter - begin);
                }
                ++iter;
            }

            return Index<uint32>::None;
        }

        EntityOrder* GetEntityOrder(const ECS::EntityId& entityId, uint32 orderIndex)
        {
            uint32 index = FindIndexOfEntityOrder(entityId, orderIndex);
            return index == Index<uint32>::None ? nullptr : &Orders[index];
        }

        const EntityOrder* GetEntityOrder(const ECS::EntityId& entityId, uint32 orderIndex) const
        {
            uint32 index = FindIndexOfEntityOrder(entityId, orderIndex);
            return index == Index<uint32>::None ? nullptr : &Orders[index];
        }

        bool InsertOrderAtIndex(const ECS::EntityId& entityId, const Order& order, uint32 index)
        {
            // Find the next invalid slot
            uint32 i = index;
            for (; i < Orders.Num(); ++i)
            {
                if (!Orders[i].IsValid())
                {
                    break;
                }
            }

            if (i == Orders.Num())
            {
                // Queue is full, can't add any new orders.
                if (Orders.IsFull())
                {
                    return false;
                }

                // Make room for a new order.
                Orders.SetNum(Orders.Num() + 1);
            }

            // Shift all orders back towards the invalid slot.
            uint32 j = i;
            for (; j > index; --j)
            {
                Orders[j] = Orders[j - 1];
            }

            PHX_ASSERT(j == index);
            Orders[j] = { entityId, order };
            return true;
        }

        TFixedArray<EntityOrder, Capacity> Orders;
        uint32 SortedNum = 0;
        uint32 NumValidOrders = 0;
    };
}
