
#include "Abilities/MoveAbility.h"

#include "BodyComponent.h"
#include "FeatureECS.h"
#include "FeatureLDS.h"
#include "FeatureSteering.h"
#include "SteeringComponent.h"
#include "Abilities/FeatureAbilities.h"
#include "Data/DataMoveAbility.h"
#include "Data/DataUnit.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::ECS;
using namespace Phoenix::Steering;
using namespace Phoenix::RTS;

namespace MoveAbilitySystemDetail
{
    struct UpdateMoveAbilityComponentJob : IBufferJob<const SeekComponent&, const TransformComponent&, MoveAbilityComponent&>
    {
        void Execute(const EntityComponentSpan<const SeekComponent&, const TransformComponent&, MoveAbilityComponent&>& span) override
        {
            PHX_PROFILE_ZONE_SCOPED_N("UpdateMoveAbilityComponentJob");

            TSharedPtr<FeatureAbilities> abilities = GetFeature<FeatureAbilities>(*World);

            for (auto && [entityId, index, seekComp, transformComp, moveComp] : span)
            {

                if (moveComp.State == EMoveAbilityState::MoveToPosition)
                {
                    auto result = moveComp.ActiveState.MoveToPosition.OnUpdate(*World, { entityId });
                    if (result != EAbilityStateResult::Continue)
                    {
                        abilities->OnActiveOrderCompleted(*World, { entityId }, result == EAbilityStateResult::Complete);
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
    : ActiveState(MoveToPositionState{})
    , State(EMoveAbilityState::Idle)
{
}

MoveAbility::MoveAbility()
    : AbilityBase("MoveAbility"_n)
{
    System = MakeShared<MoveAbilitySystem>();
}

void MoveAbility::Initialize(SessionRef session)
{
    TSharedPtr<FeatureECS> ecs = session.GetFeature<FeatureECS>();
    ecs->RegisterSystem(System);
}

void MoveAbility::Shutdown(SessionRef session)
{
    TSharedPtr<FeatureECS> ecs = session.GetFeature<FeatureECS>();
    ecs->UnregisterSystem(System);
}

void MoveAbility::OnWorldInitialize(WorldRef world)
{
    FeatureECS::RegisterComponentDefinition<MoveAbilityComponent>(world);
}

void MoveAbility::OnWorldShutdown(WorldRef world)
{
    FeatureECS::UnregisterComponentDefinition<MoveAbilityComponent>(world);
}

bool MoveAbility::AddAbility(WorldRef world, const UnitId& unit) const
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

    SeekComponent* seekComp = FeatureECS::GetOrAddComponent<SeekComponent>(world, unit);
    if (!seekComp)
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

    steeringComp->MaxSpeed = unitData.Movement.Speed.GetValue(queryContext);
    steeringComp->AvoidanceRadius = unitData.Placement.InnerRadius.GetValue(queryContext);
    steeringComp->SeparationDelay = unitData.Movement.SeparationDelay.GetValue(queryContext);
    steeringComp->SeparationRadius = unitData.Movement.SeparationRadius.GetValue(queryContext);
    steeringComp->SeparationStrength = unitData.Movement.SeparationStrength.GetValue(queryContext);

    return true;
}

bool MoveAbility::RemoveAbility(WorldRef world, const UnitId& unit) const
{
    return FeatureECS::RemoveComponent<MoveAbilityComponent>(world, unit);
}

bool MoveAbility::HasAbility(WorldConstRef world, const UnitId& unit) const
{
    return FeatureECS::GetComponent<MoveAbilityComponent>(world, unit) != nullptr;
}

uint32 MoveAbility::GetCommandPriority(WorldRef world, UnitId unit, const Command& command) const
{
    return AbilityPriority::All();
}

bool MoveAbility::ExecuteOrder(WorldRef world, UnitId unit, const Order& order) const
{
    MoveAbilityComponent* moveComp = FeatureECS::GetComponent<MoveAbilityComponent>(world, unit);

    if (order.CommandIndex == Commands::MoveToPosition)
    {
        moveComp->State = EMoveAbilityState::MoveToPosition;
        moveComp->ActiveState.MoveToPosition.OnEnter(world, unit, order.Location, 0);
    }

    return false;
}

bool MoveAbility::InterruptOrder(WorldRef world, UnitId unit, const Order& order) const
{
    MoveAbilityComponent* moveComp = FeatureECS::GetComponent<MoveAbilityComponent>(world, unit);

    if (moveComp->State == EMoveAbilityState::MoveToPosition)
    {
        moveComp->ActiveState.MoveToPosition.OnExit(world, unit);
        moveComp->State = EMoveAbilityState::Idle;
    }

    return true;
}

uint32 MoveAbility::Acquire(const Order& order) const
{
    return 0;
}

bool MoveAbility::SupportsMagicBox(const Order& order) const
{
    return false;
}
