
#include "PhoenixRTS/Abilities/Attack/AttackAbilityHandler.h"

#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/LDS/FeatureLDS.h"

#include "PhoenixPhysics/BodyComponent.h"

#include "PhoenixSteering/FeatureSteering.h"

#include "PhoenixRTS/Data/DataAttackAbility.h"
#include "PhoenixRTS/Data/DataUnit.h"
#include "PhoenixRTS/Orders/FeatureOrders.h"
#include "PhoenixRTS/TargetFiltering/TargetFiltering.h"
#include "PhoenixRTS/Units/FeatureUnit.h"
#include "PhoenixRTS/Units/UnitId.h"
#include "PhoenixRTS/Weapons/Weapons.h"
#include "PhoenixSim/Session.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::ECS;
using namespace Phoenix::Physics;
using namespace Phoenix::Steering;
using namespace Phoenix::RTS;

namespace AttackAbilitySystemDetail
{
    struct UpdateAttackAbilityComponentJob
    {
        TSharedPtr<FeatureOrders> OrdersFeature;

        void Begin(WorldRef world)
        {
            OrdersFeature = GetFeature<FeatureOrders>(world);
        }

        void Execute(WorldRef world, const EntityComponentSpan<AttackAbilityComponent&>& span) const
        {
            for (auto && [entityId, index, attackComp] : span)
            {
                if (attackComp.ActiveState != EAttackAbilityState::None)
                {
                    AbilityStateResult result = attackComp.Update(world, UnitId(entityId));
                    if (result != EAbilityStateResult::Continue)
                    {
                        OrdersFeature->OnActiveOrderCompleted(world, UnitId(entityId), result == EAbilityStateResult::Complete);
                    }
                }
            }
        }
    };
}

void AttackAbilitySystem::OnWorldUpdate(WorldRef world, const SystemUpdateArgs& args)
{
    ISystem::OnWorldUpdate(world, args);

    {
        PHX_PROFILE_ZONE_SCOPED_N("UpdateAttackAbilityComponentJob");
        AttackAbilitySystemDetail::UpdateAttackAbilityComponentJob job;
        FeatureECS::ForEachEntity(world, job);
    }
}

AttackAbilityComponent::AttackAbilityComponent()
    : States(AttackTargetState{})
    , ActiveState(EAttackAbilityState::None)
{
}

AbilityStateResult AttackAbilityComponent::Update(WorldRef world, const UnitId& unit)
{
    switch (ActiveState)
    {
        case EAttackAbilityState::AttackEntity:     return States.AttackEntity.Update(world, unit);
        case EAttackAbilityState::AttackLocation:   return States.AttackLocation.Update(world, unit);
        case EAttackAbilityState::AttackMove:       return States.AttackMove.Update(world, unit);
        case EAttackAbilityState::FollowEntity:     return States.FollowEntity.Update(world, unit);
        default:                                    return EAbilityStateResult::Fail;
    }
}

void AttackAbilityComponent::Interrupt(WorldRef world, const UnitId& unit)
{
    switch (ActiveState)
    {
        case EAttackAbilityState::AttackEntity:     States.AttackEntity.Interrupt(world, unit); break;
        case EAttackAbilityState::AttackLocation:   States.AttackLocation.Interrupt(world, unit); break;
        case EAttackAbilityState::AttackMove:       States.AttackMove.Interrupt(world, unit); break;
        case EAttackAbilityState::FollowEntity:     States.FollowEntity.Interrupt(world, unit); break;
        default:                                    break;
    }

    Exit(world, unit);
}

void AttackAbilityComponent::Exit(WorldRef world, const UnitId& unit)
{
    switch (ActiveState)
    {
        case EAttackAbilityState::AttackEntity:     States.AttackEntity.Exit(world, unit); break;
        case EAttackAbilityState::AttackLocation:   States.AttackLocation.Exit(world, unit); break;
        case EAttackAbilityState::AttackMove:       States.AttackMove.Exit(world, unit); break;
        case EAttackAbilityState::FollowEntity:     States.FollowEntity.Exit(world, unit); break;
        default:                                    break;
    }

    ActiveState = EAttackAbilityState::None;
    FeatureSteering::Stop(world, unit);
}

AttackAbilityHandler::AttackAbilityHandler()
{
    System = MakeShared<AttackAbilitySystem>();
}

FName AttackAbilityHandler::StaticGetCommandId()
{
    return "AttackAbility"_n;
}

FName AttackAbilityHandler::GetCommandId() const
{
    return StaticGetCommandId();
}

void AttackAbilityHandler::Initialize(const TSharedPtr<Phoenix::Session>& session)
{
    IAbilityHandler::Initialize(session);

    TSharedPtr<FeatureECS> ecs = session->GetFeature<FeatureECS>();
    ecs->RegisterSystem(System);
}

void AttackAbilityHandler::Shutdown()
{
    IAbilityHandler::Shutdown();

    TSharedPtr<FeatureECS> ecs = Session->GetFeature<FeatureECS>();
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
    AttackAbilityComponent* attackComp = FeatureECS::GetComponent<AttackAbilityComponent>(world, unit);
    if (!attackComp)
    {
        return false;
    }

    Data::AttackAbilityPtr attackAbility(order.OrderId);
    UnitId targetUnit = UnitId(order.TargetEntity);
    Vec2 targetLocation = order.TargetLocation;

    if (order.OrderIndex == Commands::Attack)
    {
        if (targetUnit != EntityId::Invalid)
        {
            if (ExecuteAttackTargetOrder(world, unit, targetUnit, attackAbility, *attackComp))
            {
                return true;
            }
        }
        else
        {
            if (ExecuteAttackMoveOrder(world, unit, targetLocation, attackAbility, *attackComp))
            {
                return true;
            }
        }
    }
    else if (order.OrderIndex == Commands::AttackGround)
    {
        if (ExecuteAttackGroundOrder(world, unit, targetLocation, attackAbility, *attackComp))
        {
            return true;
        }
    }

    return false;
}

bool AttackAbilityHandler::InterruptOrder(WorldRef world, const UnitId& unit, const Order& order) const
{
    if (AttackAbilityComponent* attackComp = FeatureECS::GetComponent<AttackAbilityComponent>(world, unit))
    {
        attackComp->Interrupt(world, unit);
        return true;
    }
    return false;
}

bool AttackAbilityHandler::SupportsMagicBox(const Order& order) const
{
    return false;
}

bool AttackAbilityHandler::ExecuteAttackTargetOrder(
    WorldRef world,
    const UnitId& unit,
    const UnitId& target,
    const Data::AttackAbilityPtr& attackAbility,
    AttackAbilityComponent& attackComp)
{
    if (target == unit ||
        FeatureUnit::UnitIsDead(world, target) ||
        FeatureUnit::UnitIsHidden(world, target) ||
        !FeatureUnit::UnitIsDetected(world, unit, target))
    {
        return false;
    }

    bool unitCanMove = FeatureUnit::UnitCanMove(world, unit);
    Data::WeaponPtr weapon = Weapons::FindBestEnabledWeapon(world, unit, target, unitCanMove);
    if (weapon.IsValid())
    {
        attackComp.ActiveState = EAttackAbilityState::AttackEntity;
        auto result = attackComp.States.AttackEntity.Enter(world, unit, target, weapon, attackAbility);
        return result != EAbilityStateResult::Fail;
    }

    if (unitCanMove)
    {
        attackComp.ActiveState = EAttackAbilityState::FollowEntity;
        auto result = attackComp.States.FollowEntity.Enter(world, unit, target, 0);
        return result != EAbilityStateResult::Fail;
    }

    return false;
}

bool AttackAbilityHandler::ExecuteAttackMoveOrder(
    WorldRef world,
    const UnitId& unit,
    const Vec2& target,
    const Data::AttackAbilityPtr& attackAbility,
    AttackAbilityComponent& attackComp)
{
    bool unitCanMove = FeatureUnit::UnitCanMove(world, unit);
    if (!unitCanMove)
    {
        return false;
    }

    Data::WeaponPtr weapon = Weapons::FindBestEnabledWeapon(world, unit, target, true);
    if (!weapon.IsValid())
    {
        return false;
    }

    attackComp.ActiveState = EAttackAbilityState::AttackMove;
    auto result = attackComp.States.AttackMove.Enter(world, unit, target, weapon, attackAbility);
    if (result == EAbilityStateResult::Fail)
    {
        return false;
    }

    FeatureUnit::SetTargetScanLevel(world, unit, ETargetScanLevel::Offensive);
    return true;
}

bool AttackAbilityHandler::ExecuteAttackGroundOrder(
    WorldRef world,
    const UnitId& unit,
    const Vec2& target,
    const Data::AttackAbilityPtr& attackAbility,
    AttackAbilityComponent& attackComp)
{
    bool unitCanMove = FeatureUnit::UnitCanMove(world, unit);
    Data::WeaponPtr weapon = Weapons::FindBestEnabledWeapon(world, unit, target, unitCanMove);
    if (!weapon.IsValid())
    {
        return false;
    }

    attackComp.ActiveState = EAttackAbilityState::AttackLocation;
    auto result = attackComp.States.AttackLocation.Enter(world, unit, target, weapon, attackAbility);
    return result != EAbilityStateResult::Fail;
}
