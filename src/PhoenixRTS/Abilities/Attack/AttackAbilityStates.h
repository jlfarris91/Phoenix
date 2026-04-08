#pragma once

#include "PhoenixRTS/Abilities/States/CommonAbilityStates.h"
#include "PhoenixRTS/Abilities/States/WeaponAbilityStates.h"

namespace Phoenix::RTS
{
    struct UnitId;

    namespace Data
    {
        struct AttackAbilityPtr;
    }

    enum class PHOENIX_RTS_API EAttackAbilityState : uint8
    {
        None,
        AttackEntity,
        AttackLocation,
        AttackMove,
        FollowEntity
    };

    struct PHOENIX_RTS_API AttackTargetState : TargetEntityState
    {
        FName AbilityId;
        FName WeaponId;
        Distance Range;
        Angle Arc;

        enum class EActiveState
        {
            None,
            MoveToEntity,
            FaceEntity,
            WeaponPreSwing,
            WeaponCooldown,
            WeaponSwing,
            WeaponExecute,
            WeaponBackSwing
        } ActiveState = EActiveState::None;

        union
        {
            MoveToEntityState MoveToEntity;
            FaceEntityState FaceEntity;
            WeaponPreSwingState WeaponPreSwing;
            WeaponCooldownState WeaponCooldown;
            WeaponSwingState WeaponSwing;
            WeaponExecuteState WeaponExecute;
            WeaponBackSwingState WeaponBackSwing;
        } States;

        AbilityStateResult Enter(
            WorldConstRef world,
            const UnitId& unit,
            const ECS::EntityId& target,
            const Data::WeaponPtr& weapon,
            const Data::AttackAbilityPtr& attackAbility);

        AbilityStateResult Update(WorldConstRef world, const UnitId& unit);
        void Interrupt(WorldConstRef world, const UnitId& unit);
        void Exit(WorldConstRef world, const UnitId& unit);

    private:

        AbilityStateResult SetState(WorldConstRef world, const UnitId& unit, EActiveState state);

        // Re-evaluates the available weapons and selects the best one for the target.
        // Returns true if a valid weapon was selected.
        bool ReselectWeapon(WorldConstRef world, const UnitId& unit, const ECS::EntityId& target);

        AbilityStateResult EnterActiveState(WorldConstRef world, const UnitId& unit);
        AbilityStateResult UpdateActiveState(WorldConstRef world, const UnitId& unit);
        AbilityStateResult HandleActiveStateResult(WorldConstRef world, const UnitId& unit, const AbilityStateResult& result);
        void InterruptActiveState(WorldConstRef world, const UnitId& unit);
        void ExitActiveState(WorldConstRef world, const UnitId& unit);

        AbilityStateResult HandleMoveToEntityResult(WorldConstRef world, const UnitId& unit, const AbilityStateResult& result);
        AbilityStateResult HandleFaceEntityResult(WorldConstRef world, const UnitId& unit, const AbilityStateResult& result);
        AbilityStateResult HandleWeaponPreSwingResult(WorldConstRef world, const UnitId& unit, const AbilityStateResult& result);
        AbilityStateResult HandleWeaponCooldownResult(WorldConstRef world, const UnitId& unit, const AbilityStateResult& result);
        AbilityStateResult HandleWeaponSwingResult(WorldConstRef world, const UnitId& unit, const AbilityStateResult& result);
        AbilityStateResult HandleWeaponExecuteResult(WorldConstRef world, const UnitId& unit, const AbilityStateResult& result);
        AbilityStateResult HandleWeaponBackSwingResult(WorldConstRef world, const UnitId& unit, const AbilityStateResult& result);
    };

    struct PHOENIX_RTS_API AttackLocationState : TargetLocationState
    {
        FName AbilityId;
        FName WeaponId;
        Distance Range;

        enum class EActiveState
        {
            None,
            MoveToLocation,
            FaceLocation,
            WeaponPreSwing,
            WeaponCooldown,
            WeaponSwing,
            WeaponExecute,
            WeaponBackSwing
        } ActiveState = EActiveState::None;

        union
        {
            MoveToLocationState MoveToLocation;
            FaceLocationState FaceLocation;
            WeaponPreSwingState WeaponPreSwing;
            WeaponCooldownState WeaponCooldown;
            WeaponSwingState WeaponSwing;
            WeaponExecuteState WeaponExecute;
            WeaponBackSwingState WeaponBackSwing;
        } States;

        AbilityStateResult Enter(
            WorldConstRef world,
            const UnitId& unit,
            const Vec2& target,
            const Data::WeaponPtr& weapon,
            const Data::AttackAbilityPtr& attackAbility);

        AbilityStateResult Update(WorldConstRef world, const UnitId& unit);
        void Interrupt(WorldConstRef world, const UnitId& unit);
        void Exit(WorldConstRef world, const UnitId& unit);

    private:

        AbilityStateResult SetState(WorldConstRef world, const UnitId& unit, EActiveState state);

        AbilityStateResult EnterActiveState(WorldConstRef world, const UnitId& unit);
        AbilityStateResult UpdateActiveState(WorldConstRef world, const UnitId& unit);
        void InterruptActiveState(WorldConstRef world, const UnitId& unit);
        void ExitActiveState(WorldConstRef world, const UnitId& unit);
    };

    struct PHOENIX_RTS_API AttackMoveState : TargetLocationState
    {
        FName AbilityId;
        Distance Range;

        enum class EActiveState
        {
            None,
            MoveToLocation,
        } ActiveState = EActiveState::None;

        union
        {
            MoveToLocationState MoveToLocation;
        } States;

        AbilityStateResult Enter(
            WorldConstRef world,
            const UnitId& unit,
            const Vec2& target,
            const Data::WeaponPtr& weapon,
            const Data::AttackAbilityPtr& attackAbility);

        AbilityStateResult Update(WorldConstRef world, const UnitId& unit);
        void Interrupt(WorldConstRef world, const UnitId& unit);
        void Exit(WorldConstRef world, const UnitId& unit);
    };
}