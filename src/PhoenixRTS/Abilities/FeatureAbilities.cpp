#include "PhoenixRTS/Abilities/FeatureAbilities.h"

#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/LDS/FeatureLDS.h"

#include "PhoenixRTS/Abilities/Ability.h"
#include "PhoenixRTS/Commands/Commands.h"
#include "PhoenixRTS/Data/DataUnit.h"
#include "PhoenixRTS/Orders/FeatureOrderQueue.h"
#include "PhoenixRTS/Selection/FeatureSelection.h"
#include "PhoenixRTS/Units/FeatureUnit.h"
#include "PhoenixRTS/Units/UnitId.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

void FeatureAbilities::RegisterAbilityHandler(const TSharedPtr<IAbilityHandler>& handler)
{
    AbilityIdToHandlerMap.emplace(handler->GetAbilityId(), handler);
}

bool FeatureAbilities::UnregisterAbilityHandler(const FName& abilityId)
{
    return AbilityIdToHandlerMap.erase(abilityId) > 0;
}

TSharedPtr<IAbilityHandler> FeatureAbilities::FindAbilityHandlerCached(WorldConstRef world, const FName& abilityId)
{
    TSharedPtr<IAbilityHandler> handler = FindAbilityHandler(world, abilityId);

    // Cache the handler for this ability id for faster subsequent lookups even if it is null.
    AbilityIdToHandlerMap.emplace(abilityId, handler);

    return handler;
}

TSharedPtr<IAbilityHandler> FeatureAbilities::FindAbilityHandler(WorldConstRef world, const FName& abilityId) const
{
    TSharedPtr<const ILDSQueryContext> queryContext = FeatureLDS::StaticGetWorldQueryContext(world);
    if (!queryContext)
    {
        return {};
    }

    FName currentObjectId = abilityId;
    for (;;)
    {
        auto effectHandlerIter = AbilityIdToHandlerMap.find(currentObjectId);
        if (effectHandlerIter != AbilityIdToHandlerMap.end())
        {
            return effectHandlerIter->second;
        }

        // Find the base object id
        const LDSRecord* baseRecord = queryContext->QueryRecord({ abilityId, "/base"_n }, ELDSRecordQueryFlags::Exact);
        if (!baseRecord)
        {
            return {};
        }

        currentObjectId = baseRecord->GetValueAs<FName>();
    }
}

TSharedPtr<IAbilityHandler> FeatureAbilities::StaticFindAbilityHandler(WorldConstRef world, const FName& abilityId)
{
    TSharedPtr<FeatureAbilities> feature = GetFeature<FeatureAbilities>(world);
    return feature ? feature->FindAbilityHandlerCached(world, abilityId) : TSharedPtr<IAbilityHandler>{};
}

bool FeatureAbilities::AddAbility(WorldRef world, const UnitId& unit, const FName& abilityId)
{
    TSharedPtr<IAbilityHandler> ability = StaticFindAbilityHandler(world, abilityId);
    if (!ability)
    {
        return false;
    }

    return ability->AddAbility(world, unit);
}

bool FeatureAbilities::RemoveAbility(WorldRef world, const UnitId& unit, const FName& abilityId)
{
    TSharedPtr<IAbilityHandler> ability = StaticFindAbilityHandler(world, abilityId);
    if (!ability)
    {
        return false;
    }

    return ability->RemoveAbility(world, unit);
}

bool FeatureAbilities::HasAbility(WorldConstRef world, const UnitId& unit, const FName& abilityId)
{
    TSharedPtr<IAbilityHandler> ability = StaticFindAbilityHandler(world, abilityId);
    if (!ability)
    {
        return false;
    }

    return ability->HasAbility(world, unit);
}

bool FeatureAbilities::AddAbilitiesFromData(WorldRef world, const UnitId& unit, const FName& unitData)
{
    bool success = true;

    TArray2<FName> abilities;
    GetAbilities(world, unit, abilities);

    for (const FName& abilityId : abilities)
    {
        success = AddAbility(world, unit, abilityId) && success;
        PHX_ASSERT(success);
    }

    return success;
}

uint32 FeatureAbilities::GetAbilities(WorldConstRef world, const UnitId& unit, TArray2<FName>& outAbilityIds)
{
    const ILDSQueryContext& queryContext = *FeatureLDS::StaticGetWorldQueryContext(world);

    Data::UnitPtr unitPtr(FeatureUnit::GetUnitDataId(world, unit));
    (void)unitPtr.Commands().ForEachItem(queryContext, [&](uint32, const Data::CommandPtr& command)
    {
        FName abilityId = command.Ability.GetReferenceId(queryContext);
        if (!FName::IsNoneOrEmpty(abilityId))
        {
            outAbilityIds.PushBackUnique(abilityId);
        }
    });

    return static_cast<uint32>(outAbilityIds.Num());
}

bool FeatureAbilities::StaticHandleOrder(WorldRef world, const UnitId& unit, const Order& order)
{
    TSharedPtr<FeatureAbilities> feature = GetFeature<FeatureAbilities>(world);
    return feature && feature->HandleOrder(world, unit, order);
}

bool FeatureAbilities::HandleOrder(WorldRef world, const UnitId& unit, const Order& order) const
{
    TSharedPtr<IAbilityHandler> ability = FindAbilityHandler(world, order.AbilityId);
    if (!ability)
    {
        return false;
    }

    // TODO (jfarris): should we allow an ability to deny exiting?
    if (!ExitActiveAbility(world, unit))
    {
        return false;
    }

    if (!ability->ExecuteOrder(world, unit, order))
    {
        return false;
    }

    // TODO (jfarris): broadcast event that the order was executed

    return true;
}

void FeatureAbilities::StaticOnActiveOrderCompleted(WorldRef world, const UnitId& unit, bool success)
{
    TSharedPtr<FeatureAbilities> feature = GetFeature<FeatureAbilities>(world);
    feature->OnActiveOrderCompleted(world, unit, success);
}

void FeatureAbilities::OnActiveOrderCompleted(WorldRef world, const UnitId& unit, bool success) const
{
    ExitActiveAbility(world, unit);

    // Remove the active order from the queue
    FeatureOrderQueue::RemoveOrder(world, unit, 0);

    // Handle the next head order
    uint32 index;
    if (const Order* order = FeatureOrderQueue::GetHeadOrder(world, unit, index))
    {
        StaticHandleOrder(world, unit, *order);
    }
}

void FeatureAbilities::Initialize()
{
    IFeature::Initialize();

    for (const TSharedPtr<IAbilityHandler>& ability : AbilityIdToHandlerMap | std::views::values)
    {
        ability->Initialize(*Session);
    }
}

void FeatureAbilities::Shutdown()
{
    IFeature::Shutdown();

    for (const TSharedPtr<IAbilityHandler>& ability : AbilityIdToHandlerMap | std::views::values)
    {
        ability->Shutdown(*Session);
    }
}

bool FeatureAbilities::OnHandleWorldAction(WorldRef world, const FeatureActionArgs& args)
{
    if (args.Action.Verb == "command"_n ||
        args.Action.Verb == "command_queued"_n)
    {
        return HandleCommand(world, args.Action);
    }

    if (args.Action.Verb == "smart_command"_n ||
        args.Action.Verb == "smart_command_queued"_n)
    {
        return HandleSmartCommand(world, args.Action);
    }

    return false;
}

void FeatureAbilities::OnWorldInitialize(WorldRef world)
{
    IFeature::OnWorldInitialize(world);

    for (const TSharedPtr<IAbilityHandler>& ability : AbilityIdToHandlerMap | std::views::values)
    {
        ability->OnWorldInitialize(world);
    }
}

void FeatureAbilities::OnWorldShutdown(WorldRef world)
{
    IFeature::OnWorldShutdown(world);

    for (const TSharedPtr<IAbilityHandler>& ability : AbilityIdToHandlerMap | std::views::values)
    {
        ability->OnWorldShutdown(world);
    }
}

bool FeatureAbilities::ExitActiveAbility(WorldRef world, UnitId unit) const
{
    uint32 index;
    const Order* currentOrder = FeatureOrderQueue::GetHeadOrder(world, unit, index);
    if (!currentOrder)
    {
        return false;
    }

    TSharedPtr<IAbilityHandler> ability = FindAbilityHandler(world, currentOrder->AbilityId);
    if (!ability)
    {
        return false;
    }

    return ability->InterruptOrder(world, unit, *currentOrder);
}

bool FeatureAbilities::HandleCommand(WorldRef world, const Command& command)
{
    TSharedPtr<IAbilityHandler> ability = FindAbilityHandlerCached(world, command.AbilityId);
    if (!ability)
    {
        return false;
    }

    EntityId selection = FeatureSelection::GetPlayerSelection(world, command.Sender);
    if (selection == EntityId::Invalid)
    {
        return false;
    }

    AbilityCommandContext context;
    context.AbilityId = command.AbilityId;
    context.SelectionGroupId = FName::None;
    context.LdsQueryContext = FeatureLDS::StaticGetWorldQueryContext(world);

    TArray<TTuple<UnitId, uint32>> handlers;

    // Determine how to handle the command
    FeatureECS::ForEachEntityInGroup(world, selection, [&](const EntityId& entity)
    {
        context.Unit = UnitId(entity);

        if (ability->IgnoreCommand(world, context, command))
        {
            return;
        }

        uint32 priority = ability->GetCommandPriority(world, context, command);
        if (priority != 0)
        {
            handlers.emplace_back(UnitId{entity}, priority);
        }
    });

    struct SortHandlersByPriority
    {
        bool operator()(const TTuple<UnitId, uint32>& a, const TTuple<UnitId, uint32>& b) const
        {
            return std::get<1>(a) > std::get<1>(b);
        }
    };

    if (handlers.size() > 1)
    {
        std::ranges::stable_sort(handlers, SortHandlersByPriority());
    }

    for (auto && [unit, priority] : handlers)
    {
        HandleCommand(world, unit, command);
    }

    return !handlers.empty();
}

bool FeatureAbilities::HandleSmartCommand(WorldRef world, const Command& command)
{
    EntityId selection = FeatureSelection::GetPlayerSelection(world, command.Sender);
    if (selection == EntityId::Invalid)
    {
        return false;
    }

    AbilityCommandContext context;
    context.AbilityId = FName::None;
    context.SelectionGroupId = FName::None;
    context.LdsQueryContext = FeatureLDS::StaticGetWorldQueryContext(world);

    TArray2<FName> abilityIds;

    using UnitAbilityPriority = TTuple<UnitId, uint32, TSharedPtr<IAbilityHandler>>; 
    TArray<UnitAbilityPriority> handlers;

    // Determine how to handle the command
    FeatureECS::ForEachEntityInGroup(world, selection, [&](const EntityId& entity)
    {
        context.Unit = UnitId(entity);

        abilityIds.Reset();
        GetAbilities(world, context.Unit, abilityIds);

        for (const FName& abilityId : abilityIds)
        {
            TSharedPtr<IAbilityHandler> handler = FindAbilityHandlerCached(world, abilityId);
            if (!handler)
            {
                return;
            }

            context.AbilityId = abilityId;

            if (handler->IgnoreCommand(world, context, command))
            {
                return;
            }

            uint32 priority = handler->GetSmartCommandPriority(world, context, command);
            if (priority != 0)
            {
                handlers.emplace_back(UnitId{entity}, priority, handler);
            }
        }
    });

    struct SortHandlersByPriority
    {
        bool operator()(const UnitAbilityPriority& a, const UnitAbilityPriority& b) const
        {
            return std::get<1>(a) > std::get<1>(b);
        }
    };

    if (handlers.size() > 1)
    {
        std::ranges::stable_sort(handlers, SortHandlersByPriority());
    }

    for (auto && [unit, priority, handler] : handlers)
    {
        Command command2 = command;
        command2.AbilityId = handler->GetAbilityId();
        HandleCommand(world, unit, command2);
    }

    return !handlers.empty();
}

bool FeatureAbilities::HandleCommand(WorldRef world, const UnitId& unit, const Command& command) const
{
    TSharedPtr<IAbilityHandler> ability = FindAbilityHandler(world, command.AbilityId);
    if (!ability)
    {
        return false;
    }

    Order order;
    order.AbilityId = command.AbilityId;
    order.CommandIndex = command.CommandIndex;
    order.TargetEntity = command.TargetEntity;
    order.TargetLocation = command.TargetLocation;
    order.Flags = EOrderFlags::None;

    if (HasAnyFlags(command.Flags, ECommandFlags::Smart))
    {
        SetFlagRef(order.Flags, EOrderFlags::Smart);
    }

    if (HasAnyFlags(command.Flags, ECommandFlags::Queued))
    {
        SetFlagRef(order.Flags, EOrderFlags::Queued);
    }

    if (ability->IsTransient(world, command.AbilityId))
    {
        return ability->ExecuteOrder(world, unit, order);
    }

    if (HasAnyFlags(command.Flags, ECommandFlags::Replace))
    {
        FeatureOrderQueue::ClearOrderQueue(world, unit);
    }

    if (!FeatureOrderQueue::EnqueueOrder(world, unit, order))
    {
        return false;
    }

    // TODO (jfarris): broadcast event that order was received

    return true;
}