
#include "Phoenix.Sim.RTS/Abilities/Attack/AttackAbilityHandler.h"

#include "AttackAbilityTask.h"
#include "Phoenix.Sim.ECS/FeatureECS.h"
#include "Phoenix.Sim.LDS/FeatureLDS.h"

#include "Phoenix.Sim.Physics/BodyComponent.h"

#include "Phoenix.Sim.Steering/FeatureSteering.h"

#include "Phoenix.Sim.RTS/Data/DataAttackAbility.h"
#include "Phoenix.Sim.RTS/Data/DataUnit.h"
#include "Phoenix.Sim.RTS/Orders/FeatureOrders.h"
#include "Phoenix.Sim.RTS/TargetFiltering/TargetFiltering.h"
#include "Phoenix.Sim.RTS/Units/FeatureUnit.h"
#include "Phoenix.Sim.RTS/Units/UnitId.h"
#include "Phoenix.Sim.RTS/Weapons/Weapons.h"
#include "Phoenix.Sim/Session.h"
#include "Phoenix.Sim/Tasks/FeatureTask.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::ECS;
using namespace Phoenix::Physics;
using namespace Phoenix::Steering;
using namespace Phoenix::RTS;

FName AttackAbilityHandler::StaticGetCommandId()
{
    return "AttackAbility"_n;
}

FName AttackAbilityHandler::GetCommandId() const
{
    return StaticGetCommandId();
}

bool AttackAbilityHandler::AddAbility(WorldRef world, const UnitId& unit) const
{
    if (Tasks::FeatureTask::HasTask(world, unit, StaticTypeName<AttackAbilityTask>::TypeId))
    {
        return false;
    }

    // Allocate a new attack task for the unit.
    Tasks::FeatureTask::Allocate(world, unit, AttackAbilityTask{});

    return true;
}

bool AttackAbilityHandler::RemoveAbility(WorldRef world, const UnitId& unit) const
{
    // Deallocate the attack task on the unit.
    uint32 index;
    Tasks::TaskHandle handle = Tasks::FeatureTask::GetFirstTask(world, unit, StaticTypeName<AttackAbilityTask>::TypeId, index);
    PHX_ASSERT(handle != Tasks::TaskHandle::Invalid);
    return Tasks::FeatureTask::FinishTask(world, handle);
}

bool AttackAbilityHandler::HasAbility(WorldConstRef world, const UnitId& unit) const
{
    return Tasks::FeatureTask::HasTask(world, unit, StaticTypeName<AttackAbilityTask>::TypeId);
}

uint32 AttackAbilityHandler::GetCommandPriority(
    WorldConstRef world,
    const CommandContext& context,
    const Command& command) const
{
    if (HasAnyFlags(command.Flags, ECommandFlags::Smart))
    {
        return GetSmartCommandPriority(world, context, command);
    }

    return AbilityPriority::All();
}

AcquireResult AttackAbilityHandler::AcquireOrder(
    WorldConstRef world,
    const AcquireContext& context,
    const AcquireRequest& request) const
{
    if (request.Verb == "Attack"_n)
    {
        return { context.AbilityId, Commands::Attack };
    }

    return {};
}

uint32 AttackAbilityHandler::GetSmartCommandPriority(
    WorldConstRef world,
    const CommandContext& context,
    const Command& command)
{
    const ILDSQueryContext& lds = *context.LdsQueryContext;

    Data::AttackAbilityPtr abilityDataPtr(command.CommandId);

    uint32 priority = abilityDataPtr.SmartCast().Priority().GetValue(lds);
    if (priority == 0)
    {
        return 0;
    }

    if (!FeatureECS::IsEntityValid(world, command.TargetEntity) || FeatureUnit::UnitIsDead(world, UnitId(command.TargetEntity)))
    {
        return 0;
    }

    Data::TargetFilter smartCastFilter = abilityDataPtr.SmartCast().Filter().ReadObject(lds);
    if (!TargetFiltering::PassesTargetFilter(world, smartCastFilter, context.Unit, command.TargetEntity))
    {
        return 0;
    }

    Data::UnitPtr unitDataPtr(FeatureUnit::GetUnitDataId(world, context.Unit));
    if (!unitDataPtr.IsValid())
    {
        return 0;
    }

    bool foundValidWeapon = false;
    (void)unitDataPtr.Weapons().ForEachResolvedItemObject(lds, [&](uint32, const Data::WeaponPtr& weaponPtr)
    {
        Data::TargetFilter targetFilter = weaponPtr.TargetFilter().ReadObject(lds);
        if (TargetFiltering::PassesTargetFilter(world, targetFilter, context.Unit, command.TargetEntity))
        {
            foundValidWeapon = true;
        }
    });

    if (foundValidWeapon)
    {
        return priority;
    }

    return 0;
}

bool AttackAbilityHandler::ExecuteOrder(WorldRef world, const UnitId& unit, const Order& order) const
{
    Tasks::TaskHandle handle = Tasks::FeatureTask::GetFirstTask<AttackAbilityTask>(world, unit);
    return Tasks::FeatureTask::Execute<AttackAbilityTask, bool, WorldRef, UnitId, const Order&>(world, handle, &AttackAbilityTask::ExecuteOrder, world, unit, order).GetValue(false);
}

bool AttackAbilityHandler::InterruptOrder(WorldRef world, const UnitId& unit, const Order& order) const
{
    Tasks::TaskHandle handle = Tasks::FeatureTask::GetFirstTask<AttackAbilityTask>(world, unit);
    if (handle != Tasks::TaskHandle::Invalid)
    {
        Tasks::FeatureTask::Execute<AttackAbilityTask, void, WorldRef, UnitId>(world, handle, &AttackAbilityTask::Interrupt, world, unit);
        return true;
    }
    return false;
}

bool AttackAbilityHandler::SupportsMagicBox(const Order& order) const
{
    return false;
}