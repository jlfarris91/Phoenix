#include "Phoenix.Sim.RTS/Abilities/States/WeaponAbilityStates.h"

#include "Phoenix.Sim.LDS/FeatureLDS.h"

#include "Phoenix.Sim.Steering/FeatureSteering.h"

#include "Phoenix.Sim.RTS/Data/DataWeapon.h"
#include "Phoenix.Sim.RTS/Effects/FeatureEffects.h"
#include "Phoenix.Sim.RTS/Units/FeatureUnit.h"
#include "Phoenix.Sim.RTS/Weapons/Weapons.h"

using namespace Phoenix;
using namespace Phoenix::Blackboard;
using namespace Phoenix::ECS;
using namespace Phoenix::Steering;
using namespace Phoenix::RTS;

AbilityStateResult WeaponTargetEntityState::Update(WorldRef world, const UnitId& unit) const
{
    if (!FeatureECS::IsEntityValid(world, Target) || FeatureUnit::UnitIsDead(world, UnitId(Target)))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::TargetLost };
    }

    if (!Weapons::TargetPassesFilter(world, unit, Target, WeaponId))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::TargetInvalid };
    }

    if (!Weapons::IsFacingTarget(world, unit, Target, WeaponId))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::TargetOutOfAngle };
    }

    if (Weapons::TargetIsTooClose(world, unit, Target, WeaponId))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::TargetTooClose };
    }

    if (!Weapons::TargetIsInRange(world, unit, Target, WeaponId))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::TargetOutOfRange };
    }

    return EAbilityStateResult::Continue;
}

void WeaponTargetEntityState::KeepPreSwingHot(WorldRef world, const UnitId& unit) const
{
    const LDS::ILDSQueryContext& queryContext = *LDS::FeatureLDS::StaticGetWorldQueryContext(world);
    Data::WeaponPtr weapon(WeaponId);

    Time preSwingCooldown = weapon.PreSwingCooldown().GetValue(queryContext);
    if (preSwingCooldown > 0)
    {
        FeatureECS::SetBlackboardValue(world, unit, "weapon_preswing_hot"_n, world.GetSimTime() + preSwingCooldown);
    }
}

AbilityStateResult WeaponPreSwingState::Enter(
    WorldRef world,
    const UnitId& unit,
    const Data::WeaponPtr& weapon,
    const EntityId& target)
{
    WeaponId = weapon.GetObjectId();
    Target = target;

    const LDS::ILDSQueryContext& queryContext = *LDS::FeatureLDS::StaticGetWorldQueryContext(world);

    Time preSwingHotEndTime = FeatureECS::GetBlackboardValue<Time>(world, unit, "weapon_preswing_hot"_n);

    bool isHot = world.GetSimTime() >= preSwingHotEndTime;
    Time duration = isHot
        ? weapon.PreSwingRepeat().GetValue(queryContext)
        : weapon.PreSwingInitial().GetValue(queryContext);

    Value attackSpeed = 1.0;
    CompletedTime = world.GetSimTime() + duration * attackSpeed;

    return Update(world, unit);
}

AbilityStateResult WeaponPreSwingState::Update(WorldRef world, const UnitId& unit) const
{
    AbilityStateResult result = WeaponTargetEntityState::Update(world, unit);
    if (result != EAbilityStateResult::Continue)
    {
        return result;
    }

    if (world.GetSimTime() >= CompletedTime)
    {
        return EAbilityStateResult::Complete;
    }

    return EAbilityStateResult::Continue;
}

void WeaponPreSwingState::Interrupt(WorldRef world, const UnitId& unit) const
{
    KeepPreSwingHot(world, unit);
}

void WeaponPreSwingState::Exit(WorldRef world, const UnitId& unit) const
{
    
}

AbilityStateResult WeaponCooldownState::Enter(
    WorldRef world,
    const UnitId& unit,
    const Data::WeaponPtr& weapon,
    const EntityId& target)
{
    Target = target;
    WeaponId = weapon.GetObjectId();

    const LDS::ILDSQueryContext& queryContext = *LDS::FeatureLDS::StaticGetWorldQueryContext(world);

    FName ammoId = weapon.Ammo().GetReferenceId(queryContext);
    if (Weapons::HasAmmoAndNotOnCooldown(world, unit, ammoId))
    {
        return Update(world, unit);
    }

    return EAbilityStateResult::Complete;
}

AbilityStateResult WeaponCooldownState::Update(WorldRef world, const UnitId& unit) const
{
    AbilityStateResult result = WeaponTargetEntityState::Update(world, unit);
    if (result != EAbilityStateResult::Continue)
    {
        return result;
    }

    const LDS::ILDSQueryContext& queryContext = *LDS::FeatureLDS::StaticGetWorldQueryContext(world);
    Data::WeaponPtr weapon(WeaponId);

    FName ammoId = weapon.Ammo().GetReferenceId(queryContext);
    if (Weapons::HasAmmoAndNotOnCooldown(world, unit, ammoId))
    {
        return EAbilityStateResult::Complete;
    }

    return EAbilityStateResult::Continue;
}

void WeaponCooldownState::Interrupt(WorldRef world, const UnitId& unit) const
{
    KeepPreSwingHot(world, unit);
}

void WeaponCooldownState::Exit(WorldRef world, const UnitId& unit) const
{
}

AbilityStateResult WeaponSwingState::Enter(
    WorldRef world,
    const UnitId& unit,
    const Data::WeaponPtr& weapon,
    const EntityId& target)
{
    Target = target;
    WeaponId = weapon.GetObjectId();

    const LDS::ILDSQueryContext& queryContext = *LDS::FeatureLDS::StaticGetWorldQueryContext(world);

    Time swingDuration = weapon.SwingTime().GetValue(queryContext);
    CompletedTime = world.GetSimTime() + swingDuration;

    return Update(world, unit);
}

AbilityStateResult WeaponSwingState::Update(WorldRef world, const UnitId& unit) const
{
    AbilityStateResult result = WeaponTargetEntityState::Update(world, unit);
    if (result != EAbilityStateResult::Continue)
    {
        return result;
    }

    if (world.GetSimTime() >= CompletedTime)
    {
        return EAbilityStateResult::Complete;
    }

    return EAbilityStateResult::Continue;
}

void WeaponSwingState::Interrupt(WorldRef world, const UnitId& unit) const
{
    KeepPreSwingHot(world, unit);
}

void WeaponSwingState::Exit(WorldRef world, const UnitId& unit) const
{
}

AbilityStateResult WeaponExecuteState::Enter(
    WorldRef world,
    const UnitId& unit,
    const Data::WeaponPtr& weapon,
    const EntityId& target)
{
    Target = target;
    WeaponId = weapon.GetObjectId();
    Interrupted = false;

    const LDS::ILDSQueryContext& queryContext = *LDS::FeatureLDS::StaticGetWorldQueryContext(world);

    FName ammoId = weapon.Ammo().GetReferenceId(queryContext);
    Weapons::ExpendAmmo(world, unit, ammoId);

    ExecuteEffectArgs args;
    args.Name = weapon.GetObjectId();
    args.EffectId = weapon.GetObjectId();
    args.SourceId = unit;
    args.TargetId = target;
    EffectScope = FeatureEffects::AcquireEffectScope(world, args);

    if (EffectScope == EffectScopeId::Invalid)
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::FailedToAcquireEntityScope };
    }

    FName effectObjectId = weapon.Effect().GetReferenceId(queryContext);

    if (!FeatureEffects::StaticExecuteEffect(world, EffectScope, effectObjectId))
    {
        return { EAbilityStateResult::Fail, AbilityStateReasons::FailedToExecuteEffect };
    }

    // Executing the effect could have caused this state to be interrupted
    if (Interrupted)
    {
        return EAbilityStateResult::Complete;
    }

    // TODO (jfarris): what happens if ExecuteEffect causes this state to be interrupted?

    // TODO (jfarris): emit attack event

    Data::EWeaponFlags weaponFlags = weapon.Flags().GetValue(queryContext);
    if (HasAnyFlags(weaponFlags, Data::EWeaponFlags::Channeled))
    { 
        return EAbilityStateResult::Continue;
    }

    return EAbilityStateResult::Complete;
}

AbilityStateResult WeaponExecuteState::Update(WorldRef world, const UnitId& unit) const
{
    AbilityStateResult result = WeaponTargetEntityState::Update(world, unit);
    if (result != EAbilityStateResult::Continue)
    {
        return result;
    }

    if (!FeatureEffects::IsEffectScopeChanneling(world, EffectScope))
    {
        return EAbilityStateResult::Complete;
    }

    return EAbilityStateResult::Continue;
}

void WeaponExecuteState::Interrupt(WorldRef world, const UnitId& unit)
{
    Interrupted = true;
    KeepPreSwingHot(world, unit);
    Exit(world, unit);
}

void WeaponExecuteState::Exit(WorldRef world, const UnitId& unit) const
{
    FeatureSteering::Stop(world, unit);
    FeatureEffects::EndEffectScopeChanneling(world, EffectScope);
    FeatureEffects::ReleaseEffectScope(world, EffectScope);
}

AbilityStateResult WeaponBackSwingState::Enter(
    WorldRef world,
    const UnitId& unit,
    const Data::WeaponPtr& weapon,
    const EntityId& target)
{
    Target = target;
    WeaponId = weapon.GetObjectId();

    const LDS::ILDSQueryContext& queryContext = *LDS::FeatureLDS::StaticGetWorldQueryContext(world);

    Time swingDuration = weapon.BackSwingTime().GetValue(queryContext);
    CompletedTime = world.GetSimTime() + swingDuration;

    return Update(world, unit);
}

AbilityStateResult WeaponBackSwingState::Update(WorldRef world, const UnitId& unit) const
{
    AbilityStateResult result = WeaponTargetEntityState::Update(world, unit);
    if (result != EAbilityStateResult::Continue)
    {
        return result;
    }

    if (world.GetSimTime() >= CompletedTime)
    {
        return EAbilityStateResult::Complete;
    }

    return EAbilityStateResult::Continue;
}

void WeaponBackSwingState::Interrupt(WorldRef world, const UnitId& unit) const
{
    KeepPreSwingHot(world, unit);
}

void WeaponBackSwingState::Exit(WorldRef world, const UnitId& unit) const
{
    FeatureSteering::Stop(world, unit);
}
