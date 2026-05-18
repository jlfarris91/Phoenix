
#include "PhoenixRTS/Abilities/Move/MoveAbilityHandler.h"

#include "MoveAbilityTask.h"
#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/LDS/FeatureLDS.h"

#include "PhoenixPhysics/BodyComponent.h"

#include "PhoenixSteering/FeatureSteering.h"
#include "PhoenixSteering/SteeringComponent.h"

#include "PhoenixRTS/Data/DataMoveAbility.h"
#include "PhoenixRTS/Data/DataUnit.h"
#include "PhoenixRTS/Orders/FeatureOrders.h"
#include "PhoenixRTS/Selection/FeatureSelection.h"
#include "PhoenixRTS/TargetFiltering/TargetFiltering.h"
#include "PhoenixRTS/Units/FeatureUnit.h"
#include "PhoenixRTS/Units/UnitComponent.h"
#include "PhoenixRTS/Units/UnitId.h"
#include "PhoenixSim/Session.h"
#include "PhoenixSim/Tasks/FeatureTask.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::ECS;
using namespace Phoenix::Physics;
using namespace Phoenix::Steering;
using namespace Phoenix::RTS;

FName MoveAbilityHandler::GetCommandId() const
{
    return "MoveAbility"_n;
}

bool MoveAbilityHandler::AddAbility(WorldRef world, const UnitId& unit) const
{
    if (Tasks::FeatureTask::HasTask(world, unit, StaticTypeName<MoveAbilityTask>::TypeId))
    {
        return false;
    }

    UnitComponent* unitComp = FeatureECS::GetComponent<UnitComponent>(world, unit);
    if (!unitComp)
    {
        return false;
    }

    // Allocate a new move task for the unit.
    Tasks::FeatureTask::Allocate(world, unit, MoveAbilityTask{});

    SteeringComponent* steeringComp = FeatureECS::GetOrAddComponent<SteeringComponent>(world, unit);
    if (!steeringComp)
    {
        return false;
    }

    const ILDSQueryContext& queryContext = *FeatureLDS::StaticGetWorldQueryContext(world);

    Data::UnitPtr unitData(unitComp->UnitData);

    Speed maxSpeed = unitData.Movement().Speed().GetValue(queryContext);
    Time accelerationTime = unitData.Movement().AccelerationTime().GetValue(queryContext);
    Time decelerationTime = unitData.Movement().DecelerationTime().GetValue(queryContext);

    FeatureSteering::UpdateSpeed(world, unit, { maxSpeed, accelerationTime, decelerationTime });

    steeringComp->CollisionMask = (uint32)unitData.CollisionFlags().GetValue(queryContext);
    steeringComp->InnerRadius = unitData.Placement().InnerRadius().GetValue(queryContext);
    steeringComp->OuterRadius = unitData.Placement().OuterRadius().GetValue(queryContext);
    steeringComp->TurnRateIdle = unitData.Movement().TurnRateIdle().GetValue(queryContext);
    steeringComp->TurnRateMoving = unitData.Movement().TurnRateMoving().GetValue(queryContext);
    steeringComp->AvoidanceRadius = unitData.Placement().InnerRadius().GetValue(queryContext);
    steeringComp->SeparationDelay = unitData.Movement().SeparationDelay().GetValue(queryContext);
    steeringComp->SeparationRadius = unitData.Movement().SeparationRadius().GetValue(queryContext);
    steeringComp->SeparationStrength = unitData.Movement().SeparationStrength().GetValue(queryContext);

    if (HasAnyFlags((Data::ECollisionFlags)steeringComp->CollisionMask, Data::ECollisionFlags::Ground))
    {
        FeatureECS::AddTag(world, unit, "movement_ground"_n);
    }

    return true;
}

bool MoveAbilityHandler::RemoveAbility(WorldRef world, const UnitId& unit) const
{
    // Deallocate the move task on the unit.
    uint32 index;
    Tasks::TaskHandle handle = Tasks::FeatureTask::GetFirstTask(world, unit, StaticTypeName<MoveAbilityTask>::TypeId, index);
    PHX_ASSERT(handle != Tasks::TaskHandle::Invalid);
    return Tasks::FeatureTask::FinishTask(world, handle);
}

bool MoveAbilityHandler::HasAbility(WorldConstRef world, const UnitId& unit) const
{
    return Tasks::FeatureTask::HasTask(world, unit, StaticTypeName<MoveAbilityTask>::TypeId);
}

bool MoveAbilityHandler::IgnoreCommand(
    WorldConstRef world,
    const CommandContext& context,
    const Command& command) const
{
    uint32 owningPlayer = FeatureUnit::GetOwningPlayer(world, context.Unit);
    uint32 numSelected = FeatureSelection::GetPlayerSelection(world, owningPlayer, context.SelectionGroupId);
    return numSelected > 0 && command.TargetEntity == context.Unit;
}

uint32 MoveAbilityHandler::GetCommandPriority(
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

bool MoveAbilityHandler::ExecuteOrder(WorldRef world, const UnitId& unit, const Order& order) const
{
    Tasks::TaskHandle handle = Tasks::FeatureTask::GetFirstTask<MoveAbilityTask>(world, unit);
    return Tasks::FeatureTask::Execute<MoveAbilityTask, bool, WorldRef, UnitId, const Order&>(world, handle, &MoveAbilityTask::ExecuteOrder, world, unit, order).GetValue(false);
}

bool MoveAbilityHandler::InterruptOrder(WorldRef world, const UnitId& unit, const Order& order) const
{
    Tasks::TaskHandle handle = Tasks::FeatureTask::GetFirstTask<MoveAbilityTask>(world, unit);
    if (handle != Tasks::TaskHandle::Invalid)
    {
        Tasks::FeatureTask::Execute<MoveAbilityTask, void, WorldRef, UnitId>(world, handle, &MoveAbilityTask::Interrupt, world, unit);
        return true;
    }
    return false;
}

uint32 MoveAbilityHandler::GetSmartCommandPriority(
    WorldConstRef world,
    const CommandContext& context,
    const Command& command)
{
    const ILDSQueryContext& lds = *context.LdsQueryContext;
    Data::MoveAbilityPtr dataPtr(command.CommandId);

    uint32 priority = dataPtr.SmartCast().Priority().GetValue(lds);
    if (priority == 0)
    {
        return 0;
    }

    Data::TargetFilter targetFilter = dataPtr.SmartCast().Filter().ReadObject(lds);

    const bool magicBoxed = false;
    const bool treatsAsEnemy = false;

    if (magicBoxed ||
        command.TargetEntity == EntityId::Invalid ||
        !treatsAsEnemy &&
        TargetFiltering::PassesTargetFilter(world, targetFilter, context.Unit, command.TargetEntity))
    {
        return priority;
    }

    return 0;
}
