
#include "PhoenixRTS/Orders/FeatureOrders.h"

#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/Flags.h"
#include "PhoenixSim/LDS/FeatureLDS.h"
#include "PhoenixSim/Session.h"
#include "PhoenixSim/Profiling.h"

#include "PhoenixRTS/Abilities/FeatureAbilities.h"
#include "PhoenixRTS/Orders/CommandHandler.h"
#include "PhoenixRTS/Selection/FeatureSelection.h"
#include "PhoenixRTS/Units/FeatureUnit.h"
#include "PhoenixRTS/Units/UnitId.h"
#include "PhoenixSim/Logging.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

FeatureOrders::FeatureOrders()
{
    FEATURE_WORLD_BLOCK(FeatureOrdersDynamicBlock)
    FEATURE_CHANNEL(FeatureChannels::HandleWorldAction)
    FEATURE_CHANNEL(FeatureChannels::PostWorldUpdate)
}

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

        FName baseId = baseRecord->GetValueAs<FName>();
        if (baseId == currentObjectId)
        {
            return {};
        }

        currentObjectId = baseId;
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
        StaticExecuteHeadOrder(world, unit);
    }

    return true;
}

bool FeatureOrders::InsertOrder(WorldRef world, const UnitId& unit, const Order& order, uint32 orderIndex)
{
    FeatureOrdersDynamicBlock& block = world.GetBlockRef<FeatureOrdersDynamicBlock>();
    return block.OrderQueue.InsertOrder(unit, order, orderIndex);
}

const Order* FeatureOrders::GetHeadOrder(WorldConstRef world, const UnitId& unit)
{
    const FeatureOrdersDynamicBlock& block = world.GetBlockRef<FeatureOrdersDynamicBlock>();
    return block.OrderQueue.GetFirstOrder(unit);
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

    if (index == 0)
    {
        StaticInterruptHeadOrder(world, unit);
    }

    return block.OrderQueue.RemoveOrder(unit, index);
}

uint32 FeatureOrders::ClearOrderQueue(WorldRef world, const UnitId& unit)
{
    FeatureOrdersDynamicBlock& block = world.GetBlockRef<FeatureOrdersDynamicBlock>();
    StaticInterruptHeadOrder(world, unit);
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

bool FeatureOrders::StaticRequestAcquireOrder(WorldRef world, const UnitId& unit, const AcquireRequest& request)
{
    TSharedPtr<FeatureOrders> feature = GetFeature<FeatureOrders>(world);
    return feature->RequestAcquireOrder(world, unit, request);
}

bool FeatureOrders::RequestAcquireOrder(WorldRef world, const UnitId& unit, const AcquireRequest& request)
{
    TArray2<FName> abilityIds;
    abilityIds.Reserve(8);

    // TODO (jfarris): there
    FeatureAbilities::GetAbilities(world, unit, abilityIds);

    AcquireContext context;
    context.Unit = unit;
    context.LdsQueryContext = FeatureLDS::StaticGetWorldQueryContext(world);

    Order order;
    order.Flags = EOrderFlags::Acquire;
    order.Kind = request.Kind;
    order.TargetEntity = request.TargetEntity;
    order.TargetLocation = request.TargetLocation;

    for (const FName& abilityId : abilityIds)
    {
        TSharedPtr<ICommandHandler> handler = FindCommandHandlerCached(world, abilityId);
        if (!handler)
        {
            continue;
        }

        context.AbilityId = abilityId;

        AcquireResult result = handler->AcquireOrder(world, context, request);
        if (FName::IsNoneOrEmpty(result.CommandId))
        {
            continue;
        }
        
        order.OrderId = result.CommandId;
        order.OrderIndex = result.CommandIndex;

        if (AcquireOrder(world, unit, order))
        {
            return true;
        }
    }

    return false;
}

void FeatureOrders::StaticOnActiveOrderCompleted(WorldRef world, const UnitId& unit, bool success)
{
    TSharedPtr<FeatureOrders> feature = GetFeature<FeatureOrders>(world);
    feature->OnActiveOrderCompleted(world, unit, success);
}

void FeatureOrders::OnActiveOrderCompleted(WorldRef world, const UnitId& unit, bool success)
{
    InterruptHeadOrder(world, unit);

    // Remove the active order from the queue
    RemoveOrder(world, unit, 0);

    // Handle the next head order
    ExecuteHeadOrder(world, unit);
}

void FeatureOrders::Initialize(const TSharedPtr<Phoenix::Session>& session)
{
    IFeature::Initialize(session);

    TArray2<TSharedPtr<ICommandHandler>> handlers;
    Session->GetServices2<ICommandHandler>(handlers);

    for (const TSharedPtr<ICommandHandler>& handler : handlers)
    {
        RegisterCommandHandler(handler);
    }

    TSharedPtr<FeatureECS> featureECS = session->GetFeature<FeatureECS>();
    PHX_ASSERT(featureECS);

    featureECS->OnEntityReleasing().AddStatic(&FeatureOrders::OnEntityReleasing);
}

void FeatureOrders::Shutdown()
{
    IFeature::Shutdown();

    while (!CommandIdToHandlerMap.empty())
    {
        UnregisterCommandHandler(CommandIdToHandlerMap.begin()->first);
    }

    TSharedPtr<FeatureECS> featureECS = Session->GetFeature<FeatureECS>();
    featureECS->OnEntityReleasing().RemoveAll(this);
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

void FeatureOrders::OnEntityReleasing(WorldRef world, EntityId entity)
{
    if (FeatureUnit::IsUnitEntity(world, entity))
    {
        ClearOrderQueue(world, UnitId(entity));
    }
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
    if (!FeatureUnit::UnitCanReceiveCommands(world, unit))
    {
        return false;
    }

    PHX_ASSERT(!FName::IsNoneOrEmpty(command.CommandId));

    LogVerbose("Unit {0} issued order {1} with target {2}", (uint32)unit, (uint32)command.CommandId, (uint32)command.TargetEntity);

    Order order;
    order.OrderId = command.CommandId;
    order.OrderIndex = command.CommandIndex;
    order.TargetEntity = command.TargetEntity;
    order.TargetLocation = command.TargetLocation;
    order.Flags = EOrderFlags::None;
    order.Kind = command.Kind;

    if (HasAnyFlags(command.Flags, ECommandFlags::Replace))
    {
        SetFlagRef(order.Flags, EOrderFlags::Replace);
    }

    if (HasAnyFlags(command.Flags, ECommandFlags::Queue))
    {
        SetFlagRef(order.Flags, EOrderFlags::Queued);
    }

    if (handler->IsTransient(world, order))
    {
        return ExecuteTransientOrder(world, unit, order, handler);
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

bool FeatureOrders::StaticExecuteHeadOrder(WorldRef world, const UnitId& unit)
{
    TSharedPtr<FeatureOrders> feature = GetFeature<FeatureOrders>(world);
    return feature->ExecuteHeadOrder(world, unit);
}

bool FeatureOrders::ExecuteHeadOrder(WorldRef world, const UnitId& unit)
{
    if (const Order* headOrder = GetHeadOrder(world, unit))
    {
        TSharedPtr<ICommandHandler> handler = FindCommandHandlerCached(world, headOrder->OrderId);
        if (!handler)
        {
            return false;
        }

        // Don't actively scan for new targets when executing an order.
        FeatureUnit::SetTargetScanLevel(world, unit, ETargetScanLevel::None);

        return handler->ExecuteOrder(world, unit, *headOrder);
    }

    // Return to scanning when there are no orders left in the queue.
    FeatureUnit::ResetTargetScanLevel(world, unit);
    return false;
}

bool FeatureOrders::ExecuteTransientOrder(
    WorldRef world,
    const UnitId& unit,
    const Order& order,
    const TSharedPtr<ICommandHandler>& handler)
{
    return handler->ExecuteOrder(world, unit, order);
}

bool FeatureOrders::StaticInterruptHeadOrder(WorldRef world, const UnitId& unit)
{
    TSharedPtr<FeatureOrders> feature = GetFeature<FeatureOrders>(world);
    return feature->InterruptHeadOrder(world, unit);
}

bool FeatureOrders::InterruptHeadOrder(WorldRef world, const UnitId& unit)
{
    const Order* headOrder = GetHeadOrder(world, unit);
    return headOrder && InterruptOrder(world, unit, *headOrder);
}

bool FeatureOrders::InterruptOrder(WorldRef world, const UnitId& unit, const Order& order)
{
    TSharedPtr<ICommandHandler> handler = FindCommandHandlerCached(world, order.OrderId);
    if (!handler)
    {
        return false;
    }

    return handler->InterruptOrder(world, unit, order);
}

bool FeatureOrders::AcquireOrder(WorldRef world, const UnitId& unit, const Order& order)
{
    FeatureOrdersDynamicBlock& block = world.GetBlockRef<FeatureOrdersDynamicBlock>();

    LogVerbose("Unit {0} acquired order {1} with target {2}", (uint32)unit, (uint32)order.OrderId, (uint32)order.TargetEntity);

    if (const Order* headOrder = block.OrderQueue.GetFirstOrder(unit))
    {
        // If a head order exists and was acquired then interrupt it and replace it.
        if (HasAnyFlags(headOrder->Flags, EOrderFlags::Acquire))
        {
            if (!InterruptOrder(world, unit, *headOrder))
            {
                return false;
            }

            if (!RemoveOrder(world, unit, 0))
            {
                return false;
            }
        }
        // Otherwise, just interrupt the current order.
        else if (!InterruptOrder(world, unit, *headOrder))
        {
            return false;
        }
    }

    if (!block.OrderQueue.InsertOrder(unit, order, 0))
    {
        return false;
    }

    return ExecuteHeadOrder(world, unit);
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
    if (!FeatureUnit::UnitCanReceiveCommands(world, unit))
    {
        return 0;
    }

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
