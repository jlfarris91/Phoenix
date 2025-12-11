
#include "Orders/FeatureOrderQueue.h"

#include "Profiling.h"
#include "Session.h"
#include "Abilities/FeatureAbilities.h"

using namespace Phoenix;
using namespace Phoenix::RTS;

bool FeatureOrderQueue::EnqueueOrder(WorldRef world, const UnitId& unit, const Order& order)
{
    FeatureOrderQueueDynamicBlock& block = world.GetBlockRef<FeatureOrderQueueDynamicBlock>();
    if (!block.OrderQueue.EnqueueOrder(unit, order))
    {
        return false;
    }

    if (block.OrderQueue.GetNumOrders(unit) == 1)
    {
        FeatureAbilities::StaticHandleOrder(world, unit, order);
    }

    return true;
}

bool FeatureOrderQueue::DequeueOrder(WorldRef world, const UnitId& unit, Order& outOrder)
{
    FeatureOrderQueueDynamicBlock& block = world.GetBlockRef<FeatureOrderQueueDynamicBlock>();
    return block.OrderQueue.DequeueOrder(unit, outOrder);
}

bool FeatureOrderQueue::InsertOrder(WorldRef world, const UnitId& unit, const Order& order, uint32 orderIndex)
{
    FeatureOrderQueueDynamicBlock& block = world.GetBlockRef<FeatureOrderQueueDynamicBlock>();
    return block.OrderQueue.InsertOrder(unit, order, orderIndex);
}

const Order* FeatureOrderQueue::GetHeadOrder(WorldConstRef world, const UnitId& unit, uint32& outIndex)
{
    const FeatureOrderQueueDynamicBlock& block = world.GetBlockRef<FeatureOrderQueueDynamicBlock>();
    return block.OrderQueue.GetFirstOrder(unit, outIndex);
}

const Order* FeatureOrderQueue::GetNextOrder(WorldConstRef world, const UnitId& unit, uint32 currIndex, uint32& outIndex)
{
    const FeatureOrderQueueDynamicBlock& block = world.GetBlockRef<FeatureOrderQueueDynamicBlock>();
    return block.OrderQueue.GetNextOrder(unit, currIndex, outIndex);
}

const Order* FeatureOrderQueue::GetOrder(WorldConstRef world, const UnitId& unit, uint32 orderIndex)
{
    const FeatureOrderQueueDynamicBlock& block = world.GetBlockRef<FeatureOrderQueueDynamicBlock>();
    return block.OrderQueue.GetOrder(unit, orderIndex);
}

uint32 FeatureOrderQueue::GetNumOrders(WorldConstRef world, const UnitId& unit)
{
    const FeatureOrderQueueDynamicBlock& block = world.GetBlockRef<FeatureOrderQueueDynamicBlock>();
    return block.OrderQueue.GetNumOrders(unit);
}

bool FeatureOrderQueue::HasOrders(WorldConstRef world, const UnitId& unit)
{
    const FeatureOrderQueueDynamicBlock& block = world.GetBlockRef<FeatureOrderQueueDynamicBlock>();
    uint32 index;
    return block.OrderQueue.GetFirstOrder(unit, index) != nullptr;
}

bool FeatureOrderQueue::RemoveOrder(WorldRef world, const UnitId& unit, uint32 index)
{
    FeatureOrderQueueDynamicBlock& block = world.GetBlockRef<FeatureOrderQueueDynamicBlock>();
    if (block.OrderQueue.RemoveOrder(unit, index))
    {
        return true;
    }
    return false;
}

uint32 FeatureOrderQueue::RemoveAllOrders(WorldRef world, const UnitId& unit)
{
    FeatureOrderQueueDynamicBlock& block = world.GetBlockRef<FeatureOrderQueueDynamicBlock>();
    return block.OrderQueue.RemoveAllOrders(unit);
}

void FeatureOrderQueue::OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    IFeature::OnPostWorldUpdate(world, args);

    SortOrderQueue(world);
}

void FeatureOrderQueue::SortOrderQueue(WorldRef world)
{
    PHX_PROFILE_ZONE_SCOPED;

    FeatureOrderQueueDynamicBlock& block = world.GetBlockRef<FeatureOrderQueueDynamicBlock>();
    block.OrderQueue.Sort();
}
