#include "AttackAbilityTask.h"

#include "AttackAbilityHandler.h"
#include "PhoenixRTS/Data/DataAttackAbility.h"
#include "PhoenixRTS/Data/DataWeapon.h"
#include "PhoenixRTS/Orders/FeatureOrders.h"
#include "PhoenixRTS/Units/FeatureUnit.h"
#include "PhoenixRTS/Units/UnitId.h"
#include "PhoenixRTS/Weapons/Weapons.h"
#include "PhoenixSteering/FeatureSteering.h"

using namespace Phoenix;
using namespace Phoenix::Tasks;
using namespace Phoenix::RTS;
using namespace Phoenix::Steering;

void AttackAbilityTask::OnCreate(WorldRef world, uint32 context)
{
    SetIntervalTicks(world, 1);
}

void AttackAbilityTask::OnUpdate(WorldRef world, uint32 context)
{
    UnitId unit = UnitId(context);
    
    if (ActiveState == EAttackAbilityState::Idle)
    {
        return;
    }

    AbilityStateResult result = EAbilityStateResult::Fail;

    switch (ActiveState)
    {
        case EAttackAbilityState::AttackEntity:     result = States.AttackEntity.Update(world, unit); break;
        case EAttackAbilityState::AttackLocation:   result = States.AttackLocation.Update(world, unit); break;
        case EAttackAbilityState::AttackMove:       result = States.AttackMove.Update(world, unit); break;
        case EAttackAbilityState::FollowEntity:     result = States.FollowEntity.Update(world, unit); break;
        default:                                    break;
    }

    if (result != EAbilityStateResult::Continue)
    {
        FeatureOrders::StaticOnActiveOrderCompleted(world, unit, result == EAbilityStateResult::Complete);
    }
}

void AttackAbilityTask::OnFinish(WorldRef world, uint32 context)
{
    ExitActiveState(world, UnitId(context));
}

bool AttackAbilityTask::ExecuteOrder(WorldRef world, UnitId unit, const Order& order)
{
    Data::AttackAbilityPtr attackAbility(order.OrderId);
    UnitId targetUnit = UnitId(order.TargetEntity);
    Vec2 targetLocation = order.TargetLocation;

    if (order.OrderIndex == AttackAbilityHandler::Commands::Attack)
    {
        if (targetUnit != UnitId::Invalid)
        {
            if (ExecuteAttackTargetOrder(world, unit, targetUnit, attackAbility))
            {
                return true;
            }
        }
        else
        {
            if (ExecuteAttackMoveOrder(world, unit, targetLocation, attackAbility))
            {
                return true;
            }
        }
    }
    else if (order.OrderIndex == AttackAbilityHandler::Commands::AttackGround)
    {
        if (ExecuteAttackGroundOrder(world, unit, targetLocation, attackAbility))
        {
            return true;
        }
    }

    return false;
}

bool AttackAbilityTask::ExecuteAttackTargetOrder(
    WorldRef world,
    UnitId unit,
    UnitId target,
    const Data::AttackAbilityPtr& attackAbility)
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
        ActiveState = EAttackAbilityState::AttackEntity;
        auto result = States.AttackEntity.Enter(world, unit, target, weapon, attackAbility);
        return result != EAbilityStateResult::Fail;
    }

    if (unitCanMove)
    {
        ActiveState = EAttackAbilityState::FollowEntity;
        auto result = States.FollowEntity.Enter(world, unit, target, 0);
        return result != EAbilityStateResult::Fail;
    }

    return false;
}

bool AttackAbilityTask::ExecuteAttackMoveOrder(
    WorldRef world,
    UnitId unit,
    const Vec2& target,
    const Data::AttackAbilityPtr& attackAbility)
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

    ActiveState = EAttackAbilityState::AttackMove;
    auto result = States.AttackMove.Enter(world, unit, target, weapon, attackAbility);
    if (result == EAbilityStateResult::Fail)
    {
        return false;
    }

    FeatureUnit::SetTargetScanLevel(world, unit, ETargetScanLevel::Offensive);
    return true;
}

bool AttackAbilityTask::ExecuteAttackGroundOrder(
    WorldRef world,
    UnitId unit,
    const Vec2& target,
    const Data::AttackAbilityPtr& attackAbility)
{
    bool unitCanMove = FeatureUnit::UnitCanMove(world, unit);
    Data::WeaponPtr weapon = Weapons::FindBestEnabledWeapon(world, unit, target, unitCanMove);
    if (!weapon.IsValid())
    {
        return false;
    }

    ActiveState = EAttackAbilityState::AttackLocation;
    auto result = States.AttackLocation.Enter(world, unit, target, weapon, attackAbility);
    return result != EAbilityStateResult::Fail;
}

void AttackAbilityTask::Interrupt(WorldRef world, UnitId unit)
{
    switch (ActiveState)
    {
        case EAttackAbilityState::AttackEntity:     States.AttackEntity.Interrupt(world, unit); break;
        case EAttackAbilityState::AttackLocation:   States.AttackLocation.Interrupt(world, unit); break;
        case EAttackAbilityState::AttackMove:       States.AttackMove.Interrupt(world, unit); break;
        case EAttackAbilityState::FollowEntity:     States.FollowEntity.Interrupt(world, unit); break;
        default:                                    break;
    }

    ExitActiveState(world, unit);
}

void AttackAbilityTask::ExitActiveState(WorldRef world, UnitId unit)
{
    switch (ActiveState)
    {
        case EAttackAbilityState::AttackEntity:     States.AttackEntity.Exit(world, unit); break;
        case EAttackAbilityState::AttackLocation:   States.AttackLocation.Exit(world, unit); break;
        case EAttackAbilityState::AttackMove:       States.AttackMove.Exit(world, unit); break;
        case EAttackAbilityState::FollowEntity:     States.FollowEntity.Exit(world, unit); break;
        default:                                    break;
    }

    ActiveState = EAttackAbilityState::Idle;
    FeatureSteering::Stop(world, unit);
}
