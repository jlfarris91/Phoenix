
#include "PhoenixRTS/Orders/FeatureOrders.h"

#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/Flags.h"
#include "PhoenixSim/LDS/FeatureLDS.h"
#include "PhoenixSim/Session.h"
#include "PhoenixSim/Profiling.h"

#include "PhoenixRTS/Abilities/FeatureAbilities.h"
#include "PhoenixRTS/Orders/CommandHandler.h"
#include "PhoenixRTS/Selection/FeatureSelection.h"
#include "PhoenixRTS/Units/UnitId.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

void FeatureOrders::RegisterCommandHandler(const TSharedPtr<ICommandHandler>& handler)
{
    CommandIdToHandlerMap.emplace(handler->GetCommandId(), handler);
}

bool FeatureOrders::UnregisterCommandHandler(const FName& commandId)
{
    return CommandIdToHandlerMap.erase(commandId) > 0;
}

TSharedPtr<ICommandHandler> FeatureOrders::FindCommandHandlerCached(WorldConstRef world, const FName& commandId)
{
    TSharedPtr<ICommandHandler> handler = FindCommandHandler(world, commandId);

    // Cache the handler for this ability id for faster subsequent lookups even if it is null.
    CommandIdToHandlerMap.emplace(commandId, handler);

    return handler;
}

TSharedPtr<ICommandHandler> FeatureOrders::FindCommandHandler(WorldConstRef world, const FName& commandId) const
{
    TSharedPtr<const ILDSQueryContext> queryContext = FeatureLDS::StaticGetWorldQueryContext(world);
    if (!queryContext)
    {
        return {};
    }

    FName currentObjectId = commandId;
    for (;;)
    {
        auto effectHandlerIter = CommandIdToHandlerMap.find(currentObjectId);
        if (effectHandlerIter != CommandIdToHandlerMap.end())
        {
            return effectHandlerIter->second;
        }

        // Find the base object id
        const LDSRecord* baseRecord = queryContext->QueryRecord({ commandId, "/base"_n }, ELDSRecordQueryFlags::Exact);
        if (!baseRecord)
        {
            return {};
        }

        currentObjectId = baseRecord->GetValueAs<FName>();
    }
}

TSharedPtr<ICommandHandler> FeatureOrders::StaticFindCommandHandler(WorldConstRef world, const FName& abilityId)
{
    TSharedPtr<FeatureOrders> feature = GetFeature<FeatureOrders>(world);
    return feature ? feature->FindCommandHandlerCached(world, abilityId) : TSharedPtr<ICommandHandler>{};
}

bool FeatureOrders::EnqueueOrder(WorldRef world, const UnitId& unit, const Order& order)
{
    FeatureOrdersDynamicBlock& block = world.GetBlockRef<FeatureOrdersDynamicBlock>();
    if (!block.OrderQueue.EnqueueOrder(unit, order))
    {
        return false;
    }

    if (block.OrderQueue.GetNumOrders(unit) == 1)
    {
        StaticHandleOrder(world, unit, order);
    }

    return true;
}

bool FeatureOrders::DequeueOrder(WorldRef world, const UnitId& unit, Order& outOrder)
{
    FeatureOrdersDynamicBlock& block = world.GetBlockRef<FeatureOrdersDynamicBlock>();
    return block.OrderQueue.DequeueOrder(unit, outOrder);
}

bool FeatureOrders::InsertOrder(WorldRef world, const UnitId& unit, const Order& order, uint32 orderIndex)
{
    FeatureOrdersDynamicBlock& block = world.GetBlockRef<FeatureOrdersDynamicBlock>();
    return block.OrderQueue.InsertOrder(unit, order, orderIndex);
}

const Order* FeatureOrders::GetHeadOrder(WorldConstRef world, const UnitId& unit, uint32& outIndex)
{
    const FeatureOrdersDynamicBlock& block = world.GetBlockRef<FeatureOrdersDynamicBlock>();
    return block.OrderQueue.GetFirstOrder(unit, outIndex);
}

const Order* FeatureOrders::GetNextOrder(WorldConstRef world, const UnitId& unit, uint32 currIndex, uint32& outIndex)
{
    const FeatureOrdersDynamicBlock& block = world.GetBlockRef<FeatureOrdersDynamicBlock>();
    return block.OrderQueue.GetNextOrder(unit, currIndex, outIndex);
}

const Order* FeatureOrders::GetOrder(WorldConstRef world, const UnitId& unit, uint32 orderIndex)
{
    const FeatureOrdersDynamicBlock& block = world.GetBlockRef<FeatureOrdersDynamicBlock>();
    return block.OrderQueue.GetOrder(unit, orderIndex);
}

uint32 FeatureOrders::GetNumOrders(WorldConstRef world, const UnitId& unit)
{
    const FeatureOrdersDynamicBlock& block = world.GetBlockRef<FeatureOrdersDynamicBlock>();
    return block.OrderQueue.GetNumOrders(unit);
}

bool FeatureOrders::HasOrders(WorldConstRef world, const UnitId& unit)
{
    const FeatureOrdersDynamicBlock& block = world.GetBlockRef<FeatureOrdersDynamicBlock>();
    uint32 index;
    return block.OrderQueue.GetFirstOrder(unit, index) != nullptr;
}

bool FeatureOrders::RemoveOrder(WorldRef world, const UnitId& unit, uint32 index)
{
    FeatureOrdersDynamicBlock& block = world.GetBlockRef<FeatureOrdersDynamicBlock>();
    if (block.OrderQueue.RemoveOrder(unit, index))
    {
        return true;
    }
    return false;
}

uint32 FeatureOrders::ClearOrderQueue(WorldRef world, const UnitId& unit)
{
    FeatureOrdersDynamicBlock& block = world.GetBlockRef<FeatureOrdersDynamicBlock>();
    return block.OrderQueue.RemoveAllOrders(unit);
}

TOptional<EntityId> FeatureOrders::GetHeadOrderTargetEntity(WorldConstRef world, const UnitId& unit)
{
    uint32 index;
    const Order* headOrder = GetHeadOrder(world, unit, index);
    return headOrder ? headOrder->TargetEntity : TOptional<EntityId>();
}

TOptional<Vec2> FeatureOrders::GetHeadOrderTargetLocation(WorldConstRef world, const UnitId& unit)
{
    uint32 index;
    const Order* headOrder = GetHeadOrder(world, unit, index);
    return headOrder ? headOrder->TargetLocation : TOptional<Vec2>();
}

bool FeatureOrders::StaticHandleCommand(WorldRef world, const Command& command)
{
    TSharedPtr<FeatureOrders> feature = GetFeature<FeatureOrders>(world);
    return feature && feature->HandleCommand(world, command);
}

bool FeatureOrders::HandleCommand(WorldRef world, const Command& command)
{
    TArray2<PrioritizedCommandHandler> handlers;
    if (GetHighestPriorityHandlersForSelection(world, command, handlers) == 0)
    {
        return false;
    }

    bool handled = false;

    for (auto && [unit, priority, handler] : handlers)
    {
        Command unitAbilityCommand = command;
        unitAbilityCommand.CommandId = handler->GetCommandId();
        if (HandleCommandForUnitInternal(world, unit, unitAbilityCommand, handler))
        {
            handled = true;
        }
    }

    return handled;
}

bool FeatureOrders::StaticHandleCommandForUnit(WorldRef world, const UnitId& unit, const Command& command)
{
    TSharedPtr<FeatureOrders> feature = GetFeature<FeatureOrders>(world);
    return feature && feature->HandleCommandForUnit(world, unit, command);
}

bool FeatureOrders::HandleCommandForUnit(WorldRef world, const UnitId& unit, const Command& command)
{
    CommandContext context;
    context.SelectionGroupId = FName::None;
    context.LdsQueryContext = FeatureLDS::StaticGetWorldQueryContext(world);

    PrioritizedCommandHandler prioritizedHandler;
    if (!GetHighestPriorityHandler(world, unit, context, command, prioritizedHandler))
    {
        return false;
    }

    TSharedPtr<ICommandHandler> handler = std::get<2>(prioritizedHandler);

    Command unitAbilityCommand = command;
    unitAbilityCommand.CommandId = std::get<1>(prioritizedHandler);

    return HandleCommandForUnitInternal(world, unit, unitAbilityCommand, handler);
}

bool FeatureOrders::StaticHandleOrder(WorldRef world, const UnitId& unit, const Order& order)
{
    TSharedPtr<FeatureOrders> feature = GetFeature<FeatureOrders>(world);
    return feature && feature->HandleOrder(world, unit, order);
}

bool FeatureOrders::HandleOrder(WorldRef world, const UnitId& unit, const Order& order)
{
    TSharedPtr<ICommandHandler> handler = FindCommandHandler(world, order.CommandId);
    if (!handler)
    {
        return false;
    }

    // TODO (jfarris): should we allow an ability to deny exiting?
    if (!InterruptActiveOrder(world, unit))
    {
        return false;
    }

    if (!handler->ExecuteOrder(world, unit, order))
    {
        return false;
    }

    // TODO (jfarris): broadcast event that the order was executed

    return true;
}

void FeatureOrders::StaticOnActiveOrderCompleted(WorldRef world, const UnitId& unit, bool success)
{
    TSharedPtr<FeatureOrders> feature = GetFeature<FeatureOrders>(world);
    feature->OnActiveOrderCompleted(world, unit, success);
}

void FeatureOrders::OnActiveOrderCompleted(WorldRef world, const UnitId& unit, bool success)
{
    InterruptActiveOrder(world, unit);

    // Remove the active order from the queue
    RemoveOrder(world, unit, 0);

    // Handle the next head order
    uint32 index;
    if (const Order* order = GetHeadOrder(world, unit, index))
    {
        StaticHandleOrder(world, unit, *order);
    }
}

void FeatureOrders::Initialize(const TSharedPtr<Phoenix::Session>& session)
{
    IFeature::Initialize(session);

    for (const TSharedPtr<ICommandHandler>& handler : CommandIdToHandlerMap | std::views::values)
    {
        handler->Initialize(Session);
    }
}

void FeatureOrders::Shutdown()
{
    IFeature::Shutdown();

    for (const TSharedPtr<ICommandHandler>& handler : CommandIdToHandlerMap | std::views::values)
    {
        handler->Shutdown();
    }
}

void FeatureOrders::OnWorldInitialize(WorldRef world)
{
    IFeature::OnWorldInitialize(world);

    for (const TSharedPtr<ICommandHandler>& handler : CommandIdToHandlerMap | std::views::values)
    {
        handler->OnWorldInitialize(world);
    }
}

void FeatureOrders::OnWorldShutdown(WorldRef world)
{
    IFeature::OnWorldShutdown(world);

    for (const TSharedPtr<ICommandHandler>& handler : CommandIdToHandlerMap | std::views::values)
    {
        handler->OnWorldShutdown(world);
    }
}

void FeatureOrders::OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    IFeature::OnPostWorldUpdate(world, args);

    SortOrderQueue(world);
}

bool FeatureOrders::OnHandleWorldAction(WorldRef world, const FeatureActionArgs& args)
{
    if (args.Action.Verb == "command"_n ||
        args.Action.Verb == "command_queued"_n ||
        args.Action.Verb == "command_acquired"_n ||
        args.Action.Verb == "smart_command"_n ||
        args.Action.Verb == "smart_command_queued"_n)
    {
        return HandleCommand(world, args.Action);
    }

    return false;
}

void FeatureOrders::SortOrderQueue(WorldRef world)
{
    PHX_PROFILE_ZONE_SCOPED;

    FeatureOrdersDynamicBlock& block = world.GetBlockRef<FeatureOrdersDynamicBlock>();
    block.OrderQueue.Sort();
}

bool FeatureOrders::HandleCommandForUnitInternal(
    WorldRef world,
    const UnitId& unit,
    const Command& command,
    const TSharedPtr<ICommandHandler>& handler)
{
    PHX_ASSERT(!FName::IsNoneOrEmpty(command.CommandId));

    Order order;
    order.CommandId = command.CommandId;
    order.CommandIndex = command.CommandIndex;
    order.TargetEntity = command.TargetEntity;
    order.TargetLocation = command.TargetLocation;
    order.Flags = EOrderFlags::None;

    if (HasAnyFlags(command.Flags, ECommandFlags::Smart))
    {
        SetFlagRef(order.Flags, EOrderFlags::Smart);
    }

    if (HasAnyFlags(command.Flags, ECommandFlags::Queue))
    {
        SetFlagRef(order.Flags, EOrderFlags::Queued);
    }

    if (HasAnyFlags(command.Flags, ECommandFlags::Acquire))
    {
        SetFlagRef(order.Flags, EOrderFlags::Acquire);
    }

    if (handler->IsTransient(world, order))
    {
        return handler->ExecuteOrder(world, unit, order);
    }

    if (HasAnyFlags(order.Flags, EOrderFlags::Acquire))
    {
        return false;
    }

    if (HasAnyFlags(order.Flags, EOrderFlags::Replace))
    {
        ClearOrderQueue(world, unit);
    }

    if (!EnqueueOrder(world, unit, order))
    {
        return false;
    }

    // TODO (jfarris): broadcast event that order was received

    return true;
}

bool FeatureOrders::InterruptActiveOrder(WorldRef world, const UnitId& unit)
{
    uint32 index;
    const Order* currentOrder = GetHeadOrder(world, unit, index);
    if (!currentOrder)
    {
        return false;
    }

    TSharedPtr<ICommandHandler> handler = FindCommandHandlerCached(world, currentOrder->CommandId);
    if (!handler)
    {
        return false;
    }

    return handler->InterruptOrder(world, unit, *currentOrder);
}

struct SortHandlersByPriority
{
    bool operator()(const PrioritizedCommandHandler& a, const PrioritizedCommandHandler& b) const
    {
        return std::get<1>(a) > std::get<1>(b);
    }
};

uint32 FeatureOrders::GetPrioritizedHandlers(
    WorldConstRef world,
    const UnitId& unit,
    const CommandContext& context,
    const Command& command,
    TArray2<PrioritizedCommandHandler>& outHandlers)
{
    TArray2<FName> abilityIds;
    abilityIds.Reserve(8);

    // TODO (jfarris): there
    FeatureAbilities::GetAbilities(world, unit, abilityIds);

    Command unitCommand = command;

    uint32 numHandlers = 0;
    for (const FName& abilityId : abilityIds)
    {
        TSharedPtr<ICommandHandler> handler = FindCommandHandlerCached(world, abilityId);
        if (!handler)
        {
            continue;
        }

        CommandContext unitAbilityContext = context;
        unitAbilityContext.Unit = unit;

        unitCommand.CommandId = abilityId;

        if (handler->IgnoreCommand(world, unitAbilityContext, unitCommand))
        {
            continue;
        }

        uint32 priority = handler->GetCommandPriority(world, unitAbilityContext, unitCommand);
        if (priority != 0)
        {
            outHandlers.EmplaceBack(unit, priority, handler);
            ++numHandlers;
        }
    }

    if (outHandlers.Num() > 1)
    {
        std::ranges::stable_sort(outHandlers, SortHandlersByPriority());
    }

    return numHandlers;
}

bool FeatureOrders::GetHighestPriorityHandler(
    WorldConstRef world,
    const UnitId& unit,
    const CommandContext& context,
    const Command& command,
    PrioritizedCommandHandler& outHandler)
{
    TArray2<PrioritizedCommandHandler> handlers;
    if (GetPrioritizedHandlers(world, unit, context, command, handlers) == 0)
    {
        return false;
    }

    outHandler = handlers.Front();
    return true;
}

uint32 FeatureOrders::GetHighestPriorityHandlersForSelection(
    WorldConstRef world,
    const Command& command,
    TArray2<PrioritizedCommandHandler>& outHandlers)
{
    EntityId selection = FeatureSelection::GetPlayerSelection(world, command.Sender);
    if (selection == EntityId::Invalid)
    {
        return 0;
    }

    CommandContext context;
    context.SelectionGroupId = FName::None;
    context.LdsQueryContext = FeatureLDS::StaticGetWorldQueryContext(world);

    uint32 numHandlers = 0;
    FeatureECS::ForEachEntityInGroup(world, selection, [&](const EntityId& entity)
    {
        PrioritizedCommandHandler handler;
        if (GetHighestPriorityHandler(world, UnitId(entity), context, command, handler))
        {
            outHandlers.PushBack(handler);
            ++numHandlers;
        }
    });

    return numHandlers;
}
