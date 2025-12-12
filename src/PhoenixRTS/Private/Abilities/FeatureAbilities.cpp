#include "Abilities/FeatureAbilities.h"

#include "FeatureLDS.h"
#include "Abilities/Ability.h"
#include "Data/DataUnit.h"
#include "Orders/FeatureOrderQueue.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

bool FeatureAbilities::RegisterAbility(const TSharedPtr<IAbility>& ability)
{
    if (Abilities.contains(ability->GetAbilityId()))
    {
        return false;
    }

    Abilities.emplace(ability->GetAbilityId(), ability);
    return true;
}

TSharedPtr<IAbility> FeatureAbilities::GetAbility(const FName& abilityId) const
{
    auto iter = Abilities.find(abilityId);
    return iter != Abilities.end() ? iter->second : TSharedPtr<IAbility>{};
}

TSharedPtr<IAbility> FeatureAbilities::StaticGetAbility(WorldConstRef world, const FName& abilityId)
{
    TSharedPtr<FeatureAbilities> feature = GetFeature<FeatureAbilities>(world);
    return feature ? feature->GetAbility(abilityId) : TSharedPtr<IAbility>{};
}

bool FeatureAbilities::AddAbility(WorldRef world, const UnitId& unit, const FName& abilityId)
{
    TSharedPtr<IAbility> ability = StaticGetAbility(world, abilityId);
    if (!ability)
    {
        return false;
    }

    return ability->AddAbility(world, unit);
}

bool FeatureAbilities::RemoveAbility(WorldRef world, const UnitId& unit, const FName& abilityId)
{
    TSharedPtr<IAbility> ability = StaticGetAbility(world, abilityId);
    if (!ability)
    {
        return false;
    }

    return ability->RemoveAbility(world, unit);
}

bool FeatureAbilities::HasAbility(WorldConstRef world, const UnitId& unit, const FName& abilityId)
{
    TSharedPtr<IAbility> ability = StaticGetAbility(world, abilityId);
    if (!ability)
    {
        return false;
    }

    return ability->HasAbility(world, unit);
}

bool FeatureAbilities::AddAbilitiesFromData(WorldRef world, const UnitId& unit, const FName& unitData)
{
    const LDS::ILDSQueryContext& queryContext = *LDS::FeatureLDS::StaticGetWorldQueryContext(world);

    bool success = true;

    Data::UnitPtr unitPtr(unitData);
    (void)unitPtr.Commands.ForEachItem(queryContext, [&](uint32, const Data::CommandPtr& command)
    {
        FName abilityId = command.Ability.GetReferenceId(queryContext);
        if (!FName::IsNoneOrEmpty(abilityId))
        {
            success = AddAbility(world, unit, abilityId) && success;
            PHX_ASSERT(success);
        }
    });

    return success;
}

bool FeatureAbilities::StaticHandleCommand(WorldRef world, const UnitId& unit, const Command& command)
{
    TSharedPtr<FeatureAbilities> feature = GetFeature<FeatureAbilities>(world);
    return feature && feature->HandleCommand(world, unit, command);
}

bool FeatureAbilities::HandleCommand(WorldRef world, const UnitId& unit, const Command& command) const
{
    TSharedPtr<IAbility> ability = GetAbility(command.AbilityId);
    if (!ability)
    {
        return false;
    }

    Order order;
    order.AbilityId = command.AbilityId;
    order.CommandIndex = command.CommandIndex;
    order.Target = command.TargetEntity;
    order.Location = command.TargetLocation;
    order.Flags = 0;

    if (ability->IsTransient(world, command.AbilityId))
    {
        return ability->ExecuteOrder(world, unit, order);
    }

    if (command.Type == ECommandType::Order)
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

bool FeatureAbilities::StaticHandleOrder(WorldRef world, const UnitId& unit, const Order& order)
{
    TSharedPtr<FeatureAbilities> feature = GetFeature<FeatureAbilities>(world);
    return feature && feature->HandleOrder(world, unit, order);
}

bool FeatureAbilities::HandleOrder(WorldRef world, const UnitId& unit, const Order& order) const
{
    TSharedPtr<IAbility> ability = GetAbility(order.AbilityId);
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

    for (const TSharedPtr<IAbility>& ability : Abilities | std::views::values)
    {
        ability->Initialize(*Session);
    }
}

void FeatureAbilities::Shutdown()
{
    IFeature::Shutdown();

    for (const TSharedPtr<IAbility>& ability : Abilities | std::views::values)
    {
        ability->Shutdown(*Session);
    }
}

void FeatureAbilities::OnWorldInitialize(WorldRef world)
{
    IFeature::OnWorldInitialize(world);

    for (const TSharedPtr<IAbility>& ability : Abilities | std::views::values)
    {
        ability->OnWorldInitialize(world);
    }
}

void FeatureAbilities::OnWorldShutdown(WorldRef world)
{
    IFeature::OnWorldShutdown(world);

    for (const TSharedPtr<IAbility>& ability : Abilities | std::views::values)
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

    TSharedPtr<IAbility> ability = GetAbility(currentOrder->AbilityId);
    if (!ability)
    {
        return false;
    }

    return ability->InterruptOrder(world, unit, *currentOrder);
}
