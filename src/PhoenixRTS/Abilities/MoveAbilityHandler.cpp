
#include "PhoenixRTS/Abilities/MoveAbilityHandler.h"

#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/LDS/FeatureLDS.h"

#include "PhoenixPhysics/BodyComponent.h"

#include "PhoenixSteering/FeatureSteering.h"
#include "PhoenixSteering/SteeringComponent.h"

#include "PhoenixRTS/Abilities/FeatureAbilities.h"
#include "PhoenixRTS/Data/DataMoveAbility.h"
#include "PhoenixRTS/Data/DataUnit.h"
#include "PhoenixRTS/Selection/FeatureSelection.h"
#include "PhoenixRTS/TargetFiltering/TargetFiltering.h"
#include "PhoenixRTS/Units/FeatureUnit.h"
#include "PhoenixRTS/Units/UnitId.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::ECS;
using namespace Phoenix::Physics;
using namespace Phoenix::Steering;
using namespace Phoenix::RTS;

namespace MoveAbilitySystemDetail
{
    struct UpdateMoveAbilityComponentJob : IBufferJob<const SteeringComponent&, const TransformComponent&, MoveAbilityComponent&>
    {
        void Execute(const EntityComponentSpan<const SteeringComponent&, const TransformComponent&, MoveAbilityComponent&>& span) override
        {
            PHX_PROFILE_ZONE_SCOPED_N("UpdateMoveAbilityComponentJob");

            TSharedPtr<FeatureAbilities> abilities = GetFeature<FeatureAbilities>(*World);

            for (auto && [entityId, index, seekComp, transformComp, moveComp] : span)
            {

                if (moveComp.State == EMoveAbilityState::MoveToPosition)
                {
                    auto result = moveComp.ActiveState.MoveToPosition.OnUpdate(*World, UnitId(entityId));
                    if (result != EAbilityStateResult::Continue)
                    {
                        abilities->OnActiveOrderCompleted(*World, UnitId(entityId), result == EAbilityStateResult::Complete);
                    }
                }
                if (moveComp.State == EMoveAbilityState::FollowEntity)
                {
                    auto result = moveComp.ActiveState.FollowEntity.OnUpdate(*World, UnitId(entityId));
                    if (result != EAbilityStateResult::Continue)
                    {
                        abilities->OnActiveOrderCompleted(*World, UnitId(entityId), result == EAbilityStateResult::Complete);
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
    FeatureECS::ScheduleParallel(world, job);
}

MoveAbilityComponent::MoveAbilityComponent()
    : ActiveState(MoveToLocationState{})
    , State(EMoveAbilityState::Idle)
{
}

MoveAbilityHandler::MoveAbilityHandler()
    : AbilityHandlerBase("MoveAbility"_n)
{
    System = MakeShared<MoveAbilitySystem>();
}

void MoveAbilityHandler::Initialize(SessionRef session)
{
    TSharedPtr<FeatureECS> ecs = session.GetFeature<FeatureECS>();
    ecs->RegisterSystem(System);
}

void MoveAbilityHandler::Shutdown(SessionRef session)
{
    TSharedPtr<FeatureECS> ecs = session.GetFeature<FeatureECS>();
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
    const AbilityCommandContext& context,
    const Command& command) const
{
    uint32 owningPlayer = FeatureUnit::GetOwningPlayer(world, context.Unit);
    uint32 numSelected = FeatureSelection::GetPlayerSelection(world, owningPlayer, context.SelectionGroupId);
    return numSelected > 0 && command.TargetEntity == context.Unit;
}

uint32 MoveAbilityHandler::GetCommandPriority(
    WorldConstRef world,
    const AbilityCommandContext& context,
    const Command& command) const
{
    return AbilityPriority::All();
}

uint32 MoveAbilityHandler::GetSmartCommandPriority(
    WorldConstRef world,
    const AbilityCommandContext& context,
    const Command& command) const
{
    const ILDSQueryContext& lds = *context.LdsQueryContext;
    Data::MoveAbilityPtr dataPtr(context.AbilityId);

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

bool MoveAbilityHandler::ExecuteOrder(WorldRef world, const UnitId& unit, const Order& order) const
{
    MoveAbilityComponent* moveComp = FeatureECS::GetComponent<MoveAbilityComponent>(world, unit);

    if (order.CommandIndex == Commands::Patrol)
    {
        
    }
    else 
    {
        if (order.TargetEntity != EntityId::Invalid && FeatureECS::IsEntityValid(world, order.TargetEntity))
        {
            moveComp->State = EMoveAbilityState::FollowEntity;
            moveComp->ActiveState.FollowEntity.OnEnter(world, unit, order.TargetEntity, 0);
        }
        else
        {
            moveComp->State = EMoveAbilityState::MoveToPosition;
            moveComp->ActiveState.MoveToPosition.OnEnter(world, unit, order.TargetLocation, 0);
        }
    }

    return false;
}

bool MoveAbilityHandler::InterruptOrder(WorldRef world, const UnitId& unit, const Order& order) const
{
    MoveAbilityComponent* moveComp = FeatureECS::GetComponent<MoveAbilityComponent>(world, unit);

    if (moveComp->State == EMoveAbilityState::MoveToPosition)
    {
        moveComp->ActiveState.MoveToPosition.OnExit(world, unit);
        moveComp->State = EMoveAbilityState::Idle;
    }

    if (moveComp->State == EMoveAbilityState::FollowEntity)
    {
        moveComp->ActiveState.FollowEntity.OnExit(world, unit);
        moveComp->State = EMoveAbilityState::Idle;
    }

    return true;
}

uint32 MoveAbilityHandler::Acquire(const Order& order) const
{
    return 0;
}

bool MoveAbilityHandler::SupportsMagicBox(const Order& order) const
{
    return false;
}
