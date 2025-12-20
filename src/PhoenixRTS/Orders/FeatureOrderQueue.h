
#pragma once

#include "PhoenixSim/Features.h"

#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Orders/FixedOrderQueue.h"

#ifndef PHX_RTS_ORDER_QUEUE_MAX_ORDERS
#define PHX_RTS_ORDER_QUEUE_MAX_ORDERS 4096
#endif

namespace Phoenix::ECS
{
    struct EntityId;
}

namespace Phoenix::RTS
{
    class FeatureAbilities;
    struct UnitId;
    struct Order;
    class FeatureOrderQueue;

    struct PHOENIX_RTS_API FeatureOrderQueueDynamicBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_DYNAMIC(FeatureOrderQueueDynamicBlock)

        FixedOrderQueue<PHX_RTS_ORDER_QUEUE_MAX_ORDERS> OrderQueue;
    };

    // Manages the order queues of all units in the game.
    class PHOENIX_RTS_API FeatureOrderQueue : public IFeature
    {
        PHX_FEATURE_BEGIN(FeatureOrderQueue)
            FEATURE_WORLD_BLOCK(FeatureOrderQueueDynamicBlock)
            FEATURE_CHANNEL(FeatureChannels::HandleWorldAction)
            FEATURE_CHANNEL(FeatureChannels::PostWorldUpdate)
        PHX_FEATURE_END()

    public:

        // Attempts to enqueue a new order to a unit's order queue.
        static bool EnqueueOrder(WorldRef world, const UnitId& unit, const Order& order);

        // Attempts to dequeue the head order from a unit's order queue.
        static bool DequeueOrder(WorldRef world, const UnitId& unit, Order& outOrder);

        // Attempts to insert an order at a given order index to a unit's order queue.
        static bool InsertOrder(WorldRef world, const UnitId& unit, const Order& order, uint32 orderIndex);

        // Gets the first order of a unit's order queue.
        // Use outIndex to find the next order. Note that it is an absolute index, not the unit's order index.
        static const Order* GetHeadOrder(WorldConstRef world, const UnitId& unit, uint32& outIndex);

        // Gets the next order in a unit's order queue.
        // Use outIndex to find the next order. Note that it is an absolute index, not the unit's order index.
        static const Order* GetNextOrder(WorldConstRef world, const UnitId& unit, uint32 currIndex, uint32& outIndex);

        // Gets the order at a specific index in a unit's order queue.
        // The index should be a value between 0 (the head order) and the value returned by GetNumOrders. 
        static const Order* GetOrder(WorldConstRef world, const UnitId& unit, uint32 orderIndex);

        // Gets the number of orders in a unit's order queue.
        static uint32 GetNumOrders(WorldConstRef world, const UnitId& unit);

        // Gets whether there is at least one order in a units order queue.
        static bool HasOrders(WorldConstRef world, const UnitId& unit);

        // Attempts to remove an order from a unit's order queue.
        static bool RemoveOrder(WorldRef world, const UnitId& unit, uint32 index);

        // Clears the order queue for a unit.
        static uint32 ClearOrderQueue(WorldRef world, const UnitId& unit);

        template <class TCallback>
        static void ForEachOrder(WorldConstRef world, const UnitId& unit, const TCallback& callback)
        {
            const FeatureOrderQueueDynamicBlock& block = world.GetBlockRef<FeatureOrderQueueDynamicBlock>();
            block.OrderQueue.ForEachOrder(unit, callback);
        }

    protected:

        void OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;

        static void SortOrderQueue(WorldRef world);
    };
}
