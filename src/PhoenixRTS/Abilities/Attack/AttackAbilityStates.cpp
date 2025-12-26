#include "PhoenixRTS/Abilities/Attack/AttackAbilityStates.h"

#include "PhoenixRTS/Data/DataAttackAbility.h"
#include "PhoenixRTS/Data/DataWeapon.h"
#include "PhoenixRTS/Units/FeatureUnit.h"
#include "PhoenixRTS/Weapons/Weapons.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

AbilityStateResult AttackTargetState::Enter(
    WorldRef world,
    const UnitId& unit,
    const EntityId& target,
    const Data::WeaponPtr& weapon,
    const Data::AttackAbilityPtr& attackAbility)
{
    Target = target;
    WeaponId = weapon.GetObjectId();
    AbilityId = attackAbility.GetObjectId();
    Range = Weapons::GetMaxRange(world, unit, WeaponId);
    Arc = Weapons::GetWeaponArcMin(world, unit, WeaponId);

    return SetState(world, unit, EActiveState::MoveToEntity);
}

AbilityStateResult AttackTargetState::Update(WorldRef world, const UnitId& unit)
{
    return UpdateActiveState(world, unit);
}

void AttackTargetState::Interrupt(WorldRef world, const UnitId& unit)
{
    InterruptActiveState(world, unit);
}

void AttackTargetState::Exit(WorldRef world, const UnitId& unit)
{
    ExitActiveState(world, unit);
}

AbilityStateResult AttackTargetState::SetState(WorldRef world, const UnitId& unit, EActiveState state)
{
    if (state == ActiveState)
    {
        return EAbilityStateResult::Continue;
    }

    InterruptActiveState(world, unit);

    ActiveState = state;

    return EnterActiveState(world, unit);
}

bool AttackTargetState::ReselectWeapon(WorldRef world, const UnitId& unit, const ECS::EntityId& target)
{
    bool unitCanMove = FeatureUnit::UnitCanMove(world, unit);
    Data::WeaponPtr newWeapon = Weapons::FindBestEnabledWeapon(world, unit, target, unitCanMove);
    WeaponId = newWeapon.GetObjectId();
    return newWeapon.IsValid();
}

AbilityStateResult AttackTargetState::EnterActiveState(WorldRef world, const UnitId& unit)
{
    Data::WeaponPtr weaponPtr(WeaponId);
    AbilityStateResult result;

    switch (ActiveState)
    {
        case EActiveState::MoveToEntity:    result = States.MoveToEntity.Enter(world, unit, Target, Range); break;
        case EActiveState::FaceEntity:      result = States.FaceEntity.Enter(world, unit, Target, Range, Arc); break;
        case EActiveState::WeaponPreSwing:  result = States.WeaponPreSwing.Enter(world, unit, weaponPtr, Target); break;
        case EActiveState::WeaponCooldown:  result = States.WeaponCooldown.Enter(world, unit, weaponPtr, Target); break;
        case EActiveState::WeaponSwing:     result = States.WeaponSwing.Enter(world, unit, weaponPtr, Target); break;
        case EActiveState::WeaponExecute:   result = States.WeaponExecute.Enter(world, unit, weaponPtr, Target); break;
        case EActiveState::WeaponBackSwing: result = States.WeaponBackSwing.Enter(world, unit, weaponPtr, Target); break;
        case EActiveState::None:            return result;
    }

    return HandleActiveStateResult(world, unit, result);
}

AbilityStateResult AttackTargetState::UpdateActiveState(WorldRef world, const UnitId& unit)
{
    AbilityStateResult result;

    switch (ActiveState)
    {
        case EActiveState::MoveToEntity:    result = States.MoveToEntity.Update(world, unit); break;
        case EActiveState::FaceEntity:      result = States.FaceEntity.Update(world, unit); break;
        case EActiveState::WeaponPreSwing:  result = States.WeaponPreSwing.Update(world, unit); break;
        case EActiveState::WeaponCooldown:  result = States.WeaponCooldown.Update(world, unit); break;
        case EActiveState::WeaponSwing:     result = States.WeaponSwing.Update(world, unit); break;
        case EActiveState::WeaponExecute:   result = States.WeaponExecute.Update(world, unit); break;
        case EActiveState::WeaponBackSwing: result = States.WeaponBackSwing.Update(world, unit); break;
        case EActiveState::None:            return result;
    }

    return HandleActiveStateResult(world, unit, result);
}

AbilityStateResult AttackTargetState::HandleActiveStateResult(
    WorldRef world,
    const UnitId& unit,
    const AbilityStateResult& result)
{
    AbilityStateResult newResult;

    switch (ActiveState)
    {
        case EActiveState::MoveToEntity:    newResult = HandleMoveToEntityResult(world, unit, result); break;
        case EActiveState::FaceEntity:      newResult = HandleFaceEntityResult(world, unit, result); break;
        case EActiveState::WeaponPreSwing:  newResult = HandleWeaponPreSwingResult(world, unit, result); break;
        case EActiveState::WeaponCooldown:  newResult = HandleWeaponCooldownResult(world, unit, result); break;
        case EActiveState::WeaponSwing:     newResult = HandleWeaponSwingResult(world, unit, result); break;
        case EActiveState::WeaponExecute:   newResult = HandleWeaponExecuteResult(world, unit, result); break;
        case EActiveState::WeaponBackSwing: newResult = HandleWeaponBackSwingResult(world, unit, result); break;
        case EActiveState::None:            return result;
    }

    // Handle general failure cases here
    if (newResult.Result == EAbilityStateResult::Fail)
    {
        switch (newResult.Reason)
        {
            default:
            case AbilityStateReasons::TargetTooClose:
            case AbilityStateReasons::TargetLost:
            {
                // Continues to end the order
                break;
            }
            case AbilityStateReasons::TargetInvalid:
            {
                // Re-evaluate the same target using the available enabled weapons.
                if (ReselectWeapon(world, unit, Target))
                {
                    // If we found a valid weapon then start moving to the target.
                    return SetState(world, unit, EActiveState::MoveToEntity);
                }
                break;
            }
            case AbilityStateReasons::TargetOutOfRange:
            {
                newResult = SetState(world, unit, EActiveState::MoveToEntity);
                break;
            }
            case AbilityStateReasons::TargetOutOfAngle:
            {
                newResult = SetState(world, unit, EActiveState::FaceEntity);
                break;
            }
        }
    }

    return newResult;
}

void AttackTargetState::InterruptActiveState(WorldRef world, const UnitId& unit)
{
    switch (ActiveState)
    {
        case EActiveState::MoveToEntity:    States.MoveToEntity.Interrupt(world, unit); break;
        case EActiveState::FaceEntity:      States.FaceEntity.Interrupt(world, unit); break;
        case EActiveState::WeaponPreSwing:  States.WeaponPreSwing.Interrupt(world, unit); break;
        case EActiveState::WeaponCooldown:  States.WeaponCooldown.Interrupt(world, unit); break;
        case EActiveState::WeaponSwing:     States.WeaponSwing.Interrupt(world, unit); break;
        case EActiveState::WeaponExecute:   States.WeaponExecute.Interrupt(world, unit); break;
        case EActiveState::WeaponBackSwing: States.WeaponBackSwing.Interrupt(world, unit); break;
        case EActiveState::None:            break;
    }
}

void AttackTargetState::ExitActiveState(WorldRef world, const UnitId& unit)
{
    switch (ActiveState)
    {
        case EActiveState::MoveToEntity:    States.MoveToEntity.Exit(world, unit); break;
        case EActiveState::FaceEntity:      States.FaceEntity.Exit(world, unit); break;
        case EActiveState::WeaponPreSwing:  States.WeaponPreSwing.Exit(world, unit); break;
        case EActiveState::WeaponCooldown:  States.WeaponCooldown.Exit(world, unit); break;
        case EActiveState::WeaponSwing:     States.WeaponSwing.Exit(world, unit); break;
        case EActiveState::WeaponExecute:   States.WeaponExecute.Exit(world, unit); break;
        case EActiveState::WeaponBackSwing: States.WeaponBackSwing.Exit(world, unit); break;
        case EActiveState::None:            break;
    }
}

AbilityStateResult AttackTargetState::HandleMoveToEntityResult(WorldRef world, const UnitId& unit, const AbilityStateResult& result)
{
    if (result.Result == EAbilityStateResult::Complete)
    {
        return SetState(world, unit, EActiveState::FaceEntity);
    }

    return result;
}

AbilityStateResult AttackTargetState::HandleFaceEntityResult(
    WorldRef world,
    const UnitId& unit,
    const AbilityStateResult& result)
{
    if (result.Result == EAbilityStateResult::Complete)
    {
        return SetState(world, unit, EActiveState::WeaponPreSwing);
    }

    return result;
}

AbilityStateResult AttackTargetState::HandleWeaponPreSwingResult(
    WorldRef world,
    const UnitId& unit,
    const AbilityStateResult& result)
{
    if (result.Result == EAbilityStateResult::Complete)
    {
        return SetState(world, unit, EActiveState::WeaponCooldown);
    }

    return result;
}

AbilityStateResult AttackTargetState::HandleWeaponCooldownResult(
    WorldRef world,
    const UnitId& unit,
    const AbilityStateResult& result)
{
    if (result.Result == EAbilityStateResult::Complete)
    {
        return SetState(world, unit, EActiveState::WeaponSwing);
    }

    return result;
}

AbilityStateResult AttackTargetState::HandleWeaponSwingResult(
    WorldRef world,
    const UnitId& unit,
    const AbilityStateResult& result)
{
    if (result.Result == EAbilityStateResult::Complete)
    {
        return SetState(world, unit, EActiveState::WeaponExecute);
    }

    return result;
}

AbilityStateResult AttackTargetState::HandleWeaponExecuteResult(
    WorldRef world,
    const UnitId& unit,
    const AbilityStateResult& result)
{
    if (result.Result == EAbilityStateResult::Complete)
    {
        return SetState(world, unit, EActiveState::WeaponBackSwing);
    }

    return result;
}

AbilityStateResult AttackTargetState::HandleWeaponBackSwingResult(
    WorldRef world,
    const UnitId& unit,
    const AbilityStateResult& result)
{
    if (result.Result == EAbilityStateResult::Complete)
    {
        // Try to retarget the same target with the current best weapon.
        if (ReselectWeapon(world, unit, Target))
        {
            return SetState(world, unit, EActiveState::WeaponPreSwing);
        }
    }

    return result;
}

AbilityStateResult AttackLocationState::Enter(
    WorldRef world,
    const UnitId& unit,
    const Vec2& target,
    const Data::WeaponPtr& weapon,
    const Data::AttackAbilityPtr& attackAbility)
{
    return EAbilityStateResult::Complete;
}

AbilityStateResult AttackLocationState::Update(WorldRef world, const UnitId& unit)
{
    return EAbilityStateResult::Complete;
}

void AttackLocationState::Interrupt(WorldRef world, const UnitId& unit)
{
}

void AttackLocationState::Exit(WorldRef world, const UnitId& unit)
{
}

AbilityStateResult AttackLocationState::SetState(WorldRef world, const UnitId& unit, EActiveState state)
{
    return EAbilityStateResult::Complete;
}

AbilityStateResult AttackLocationState::EnterActiveState(WorldRef world, const UnitId& unit)
{
    return EAbilityStateResult::Complete;
}

AbilityStateResult AttackLocationState::UpdateActiveState(WorldRef world, const UnitId& unit)
{
    return EAbilityStateResult::Complete;
}

void AttackLocationState::InterruptActiveState(WorldRef world, const UnitId& unit)
{
}

void AttackLocationState::ExitActiveState(WorldRef world, const UnitId& unit)
{
}

AbilityStateResult AttackMoveState::Enter(
    WorldRef world,
    const UnitId& unit,
    const Vec2& target,
    const Data::WeaponPtr& weapon,
    const Data::AttackAbilityPtr& attackAbility)
{
    return EAbilityStateResult::Complete;
}

AbilityStateResult AttackMoveState::Update(WorldRef world, const UnitId& unit)
{
    return EAbilityStateResult::Complete;
}

void AttackMoveState::Interrupt(WorldRef world, const UnitId& unit)
{
}

void AttackMoveState::Exit(WorldRef world, const UnitId& unit)
{
}

AbilityStateResult AttackMoveState::SetState(WorldRef world, const UnitId& unit, EActiveState state)
{
    return EAbilityStateResult::Complete;
}

AbilityStateResult AttackMoveState::EnterActiveState(WorldRef world, const UnitId& unit)
{
    return EAbilityStateResult::Complete;
}

AbilityStateResult AttackMoveState::UpdateActiveState(WorldRef world, const UnitId& unit)
{
    return EAbilityStateResult::Complete;
}

void AttackMoveState::InterruptActiveState(WorldRef world, const UnitId& unit)
{
}

void AttackMoveState::ExitActiveState(WorldRef world, const UnitId& unit)
{
}
