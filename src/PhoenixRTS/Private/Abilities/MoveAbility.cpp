
#include "Abilities/MoveAbility.h"

#include "FeatureECS.h"
#include "FeatureLDS.h"
#include "SteeringComponent.h"
#include "Data/DataMoveAbility.h"
#include "Data/DataUnit.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::ECS;
using namespace Phoenix::Steering;
using namespace Phoenix::RTS;

namespace MoveAbilitySystemDetail
{
    struct UpdateMoveAbilityComponentJob : IBufferJob<MoveAbilityComponent&>
    {
        void Execute(const EntityComponentSpan<MoveAbilityComponent&>& span) override
        {
            PHX_PROFILE_ZONE_SCOPED_N("UpdateMoveAbilityComponentJob");

            for (auto && [entityId, index, moveComp] : span)
            {
                
            }
        }
    };
}

void MoveAbilitySystem::OnWorldUpdate(WorldRef world, const SystemUpdateArgs& args)
{
    ISystem::OnWorldUpdate(world, args);
}

MoveAbilityComponent::MoveAbilityComponent()
    : ActiveState(MoveToPositionState{})
    , State(EMoveAbilityState::MoveToPosition)
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

bool MoveAbility::AddAbility(WorldRef world, const UnitId& unit)
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

bool MoveAbility::RemoveAbility(WorldRef world, const UnitId& unit)
{
    return FeatureECS::RemoveComponent<MoveAbilityComponent>(world, unit);
}

bool MoveAbility::HasAbility(WorldConstRef world, const UnitId& unit)
{
    return false;
}

uint32 MoveAbility::HandleOrder(EOrderType type, const Order& order)
{
    return 0;
}

uint32 MoveAbility::GetPriority(const Order& order)
{
    return 0;
}

uint32 MoveAbility::Acquire(const Order& order)
{
    return 0;
}

bool MoveAbility::SupportsMagicBox(const Order& order)
{
    return false;
}
