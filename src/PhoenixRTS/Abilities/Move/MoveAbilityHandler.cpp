
#include "PhoenixRTS/Abilities/Move/MoveAbilityHandler.h"

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

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::ECS;
using namespace Phoenix::Physics;
using namespace Phoenix::Steering;
using namespace Phoenix::RTS;

namespace MoveAbilitySystemDetail
{
    struct UpdateMoveAbilityComponentJob : IBufferJob<MoveAbilityComponent&>
    {
        void Execute(const EntityComponentSpan<MoveAbilityComponent&>& span) override
        {
            PHX_PROFILE_ZONE_SCOPED_N("UpdateMoveAbilityComponentJob");

            TSharedPtr<FeatureOrders> ordersFeature = GetFeature<FeatureOrders>(*World);

            for (auto && [entityId, index, moveComp] : span)
            {
                if (moveComp.ActiveState != EMoveAbilityState::Idle)
                {
                    AbilityStateResult result = moveComp.Update(*World, UnitId(entityId));
                    if (result != EAbilityStateResult::Continue)
                    {
                        ordersFeature->OnActiveOrderCompleted(*World, UnitId(entityId), result == EAbilityStateResult::Complete);
                    }
                }
            }
        }
    };
}

void MoveAbilitySystem::OnWorldUpdate(WorldRef world, const SystemUpdateArgs& args)
{
    ISystem::OnWorldUpdate(world, args);

    MoveAbilitySystemDetail::UpdateMoveAbilityComponentJob job;
    FeatureECS::Schedule(world, job);
}

MoveAbilityComponent::MoveAbilityComponent()
    : States(MoveToLocationState{})
    , ActiveState(EMoveAbilityState::Idle)
{
}

AbilityStateResult MoveAbilityComponent::Update(WorldRef world, const UnitId& unit)
{
    switch (ActiveState)
    {
        case EMoveAbilityState::MoveToPosition: return States.MoveToPosition.Update(world, unit);
        case EMoveAbilityState::FollowEntity:   return States.FollowEntity.Update(world, unit);
        default:                                return EAbilityStateResult::Fail;
    }
}

void MoveAbilityComponent::Interrupt(WorldRef world, const UnitId& unit)
{
    switch (ActiveState)
    {
        case EMoveAbilityState::MoveToPosition: States.MoveToPosition.Interrupt(world, unit); break;
        case EMoveAbilityState::FollowEntity:   States.FollowEntity.Interrupt(world, unit); break;
        default:                                break;
    }

    Exit(world, unit);
}

void MoveAbilityComponent::Exit(WorldRef world, const UnitId& unit)
{
    switch (ActiveState)
    {
        case EMoveAbilityState::MoveToPosition: States.MoveToPosition.Exit(world, unit); break;
        case EMoveAbilityState::FollowEntity:   States.FollowEntity.Exit(world, unit); break;
        default:                                break;
    }

    ActiveState = EMoveAbilityState::Idle;
    FeatureSteering::Stop(world, unit);
}

MoveAbilityHandler::MoveAbilityHandler()
{
    System = MakeShared<MoveAbilitySystem>();
}

FName MoveAbilityHandler::GetCommandId() const
{
    return "MoveAbility"_n;
}

void MoveAbilityHandler::Initialize(const TSharedPtr<Phoenix::Session>& session)
{
    IAbilityHandler::Initialize(session);

    TSharedPtr<FeatureECS> ecs = session->GetFeature<FeatureECS>();
    ecs->RegisterSystem(System);
}

void MoveAbilityHandler::Shutdown()
{
    IAbilityHandler::Shutdown();

    TSharedPtr<FeatureECS> ecs = Session->GetFeature<FeatureECS>();
    ecs->UnregisterSystem(System);
}

void MoveAbilityHandler::OnWorldInitialize(WorldRef world)
{
    FeatureECS::RegisterComponentDefinition<MoveAbilityComponent>(world);
}

void MoveAbilityHandler::OnWorldShutdown(WorldRef world)
{
    FeatureECS::UnregisterComponentDefinition<MoveAbilityComponent>(world);
}

bool MoveAbilityHandler::AddAbility(WorldRef world, const UnitId& unit) const
{
    UnitComponent* unitComp = FeatureECS::GetComponent<UnitComponent>(world, unit);
    if (!unitComp)
    {
        return false;
    }

    MoveAbilityComponent* moveComp = FeatureECS::GetOrAddComponent<MoveAbilityComponent>(world, unit);
    if (!moveComp)
    {
        return false;
    }

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
    return FeatureECS::RemoveComponent<MoveAbilityComponent>(world, unit);
}

bool MoveAbilityHandler::HasAbility(WorldConstRef world, const UnitId& unit) const
{
    return FeatureECS::GetComponent<MoveAbilityComponent>(world, unit) != nullptr;
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
    MoveAbilityComponent* moveComp = FeatureECS::GetComponent<MoveAbilityComponent>(world, unit);

    if (order.OrderIndex == Commands::Patrol)
    {
        // TODO (jfarris): implement patrol states
    }
    else 
    {
        if (order.TargetEntity != EntityId::Invalid && FeatureECS::IsEntityValid(world, order.TargetEntity))
        {
            moveComp->ActiveState = EMoveAbilityState::FollowEntity;
            moveComp->States.FollowEntity.Enter(world, unit, order.TargetEntity, 0);
        }
        else
        {
            moveComp->ActiveState = EMoveAbilityState::MoveToPosition;
            moveComp->States.MoveToPosition.Enter(world, unit, order.TargetLocation, 0);
        }
    }

    return false;
}

bool MoveAbilityHandler::InterruptOrder(WorldRef world, const UnitId& unit, const Order& order) const
{
    if (MoveAbilityComponent* moveComp = FeatureECS::GetComponent<MoveAbilityComponent>(world, unit))
    {
        moveComp->Interrupt(world, unit);
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
