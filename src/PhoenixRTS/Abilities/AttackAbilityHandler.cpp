
#include "PhoenixRTS/Abilities/AttackAbilityHandler.h"

#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/LDS/FeatureLDS.h"

#include "PhoenixPhysics/BodyComponent.h"

#include "PhoenixSteering/FeatureSteering.h"
#include "PhoenixSteering/SteeringComponent.h"

#include "PhoenixRTS/Abilities/FeatureAbilities.h"
#include "PhoenixRTS/Data/DataAttackAbility.h"
#include "PhoenixRTS/Data/DataUnit.h"
#include "PhoenixRTS/TargetFiltering/TargetFiltering.h"
#include "PhoenixRTS/Units/FeatureUnit.h"
#include "PhoenixRTS/Units/UnitId.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::ECS;
using namespace Phoenix::Physics;
using namespace Phoenix::Steering;
using namespace Phoenix::RTS;

namespace AttackAbilitySystemDetail
{
    struct UpdateAttackAbilityComponentJob : IBufferJob<const SteeringComponent&, const TransformComponent&, AttackAbilityComponent&>
    {
        void Execute(const EntityComponentSpan<const SteeringComponent&, const TransformComponent&, AttackAbilityComponent&>& span) override
        {
            PHX_PROFILE_ZONE_SCOPED_N("UpdateAttackAbilityComponentJob");

            TSharedPtr<FeatureAbilities> abilities = GetFeature<FeatureAbilities>(*World);

            for (auto && [entityId, index, seekComp, transformComp, moveComp] : span)
            {

                if (moveComp.State == EAttackAbilityState::MoveToPosition)
                {
                    auto result = moveComp.ActiveState.MoveToPosition.OnUpdate(*World, UnitId(entityId));
                    if (result != EAbilityStateResult::Continue)
                    {
                        abilities->OnActiveOrderCompleted(*World, UnitId(entityId), result == EAbilityStateResult::Complete);
                    }
                }

            }
        }
    };
}

void AttackAbilitySystem::OnWorldUpdate(WorldRef world, const SystemUpdateArgs& args)
{
    ISystem::OnWorldUpdate(world, args);

    AttackAbilitySystemDetail::UpdateAttackAbilityComponentJob job;
    FeatureECS::ScheduleParallel(world, job);
}

AttackAbilityComponent::AttackAbilityComponent()
    : ActiveState(MoveToLocationState{})
    , State(EAttackAbilityState::Idle)
{
}

AttackAbilityHandler::AttackAbilityHandler()
    : AbilityHandlerBase("AttackAbility"_n)
{
    System = MakeShared<AttackAbilitySystem>();
}

void AttackAbilityHandler::Initialize(SessionRef session)
{
    TSharedPtr<FeatureECS> ecs = session.GetFeature<FeatureECS>();
    ecs->RegisterSystem(System);
}

void AttackAbilityHandler::Shutdown(SessionRef session)
{
    TSharedPtr<FeatureECS> ecs = session.GetFeature<FeatureECS>();
    ecs->UnregisterSystem(System);
}

void AttackAbilityHandler::OnWorldInitialize(WorldRef world)
{
    FeatureECS::RegisterComponentDefinition<AttackAbilityComponent>(world);
}

void AttackAbilityHandler::OnWorldShutdown(WorldRef world)
{
    FeatureECS::UnregisterComponentDefinition<AttackAbilityComponent>(world);
}

bool AttackAbilityHandler::AddAbility(WorldRef world, const UnitId& unit) const
{
    AttackAbilityComponent* attackComp = FeatureECS::GetOrAddComponent<AttackAbilityComponent>(world, unit);
    if (!attackComp)
    {
        return false;
    }

    return true;
}

bool AttackAbilityHandler::RemoveAbility(WorldRef world, const UnitId& unit) const
{
    return FeatureECS::RemoveComponent<AttackAbilityComponent>(world, unit);
}

bool AttackAbilityHandler::HasAbility(WorldConstRef world, const UnitId& unit) const
{
    return FeatureECS::GetComponent<AttackAbilityComponent>(world, unit) != nullptr;
}

uint32 AttackAbilityHandler::GetCommandPriority(
    WorldConstRef world,
    const AbilityCommandContext& context,
    const Command& command) const
{
    return AbilityPriority::All();
}

uint32 AttackAbilityHandler::GetSmartCommandPriority(
    WorldConstRef world,
    const AbilityCommandContext& context,
    const Command& command) const
{
    const ILDSQueryContext& lds = *context.LdsQueryContext;

    Data::AttackAbilityPtr abilityDataPtr(context.AbilityId);

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
    AttackAbilityComponent* moveComp = FeatureECS::GetComponent<AttackAbilityComponent>(world, unit);

    if (order.CommandIndex == Commands::Attack)
    {
        moveComp->State = EAttackAbilityState::MoveToPosition;
        moveComp->ActiveState.MoveToPosition.OnEnter(world, unit, order.TargetLocation, 0);
    }

    return false;
}

bool AttackAbilityHandler::InterruptOrder(WorldRef world, const UnitId& unit, const Order& order) const
{
    AttackAbilityComponent* moveComp = FeatureECS::GetComponent<AttackAbilityComponent>(world, unit);

    if (moveComp->State == EAttackAbilityState::MoveToPosition)
    {
        moveComp->ActiveState.MoveToPosition.OnExit(world, unit);
        moveComp->State = EAttackAbilityState::Idle;
    }

    return true;
}

uint32 AttackAbilityHandler::Acquire(const Order& order) const
{
    return 0;
}

bool AttackAbilityHandler::SupportsMagicBox(const Order& order) const
{
    return false;
}
