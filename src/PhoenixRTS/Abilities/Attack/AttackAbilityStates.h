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
            WorldRef world,
            const UnitId& unit,
            const ECS::EntityId& target,
            const Data::WeaponPtr& weapon,
            const Data::AttackAbilityPtr& attackAbility);

        AbilityStateResult Update(WorldRef world, const UnitId& unit);
        void Interrupt(WorldRef world, const UnitId& unit);
        void Exit(WorldRef world, const UnitId& unit);

    private:

        AbilityStateResult SetState(WorldRef world, const UnitId& unit, EActiveState state);

        // Re-evaluates the available weapons and selects the best one for the target.
        // Returns true if a valid weapon was selected.
        bool ReselectWeapon(WorldRef world, const UnitId& unit, const ECS::EntityId& target);

        AbilityStateResult EnterActiveState(WorldRef world, const UnitId& unit);
        AbilityStateResult UpdateActiveState(WorldRef world, const UnitId& unit);
        AbilityStateResult HandleActiveStateResult(WorldRef world, const UnitId& unit, const AbilityStateResult& result);
        void InterruptActiveState(WorldRef world, const UnitId& unit);
        void ExitActiveState(WorldRef world, const UnitId& unit);

        AbilityStateResult HandleMoveToEntityResult(WorldRef world, const UnitId& unit, const AbilityStateResult& result);
        AbilityStateResult HandleFaceEntityResult(WorldRef world, const UnitId& unit, const AbilityStateResult& result);
        AbilityStateResult HandleWeaponPreSwingResult(WorldRef world, const UnitId& unit, const AbilityStateResult& result);
        AbilityStateResult HandleWeaponCooldownResult(WorldRef world, const UnitId& unit, const AbilityStateResult& result);
        AbilityStateResult HandleWeaponSwingResult(WorldRef world, const UnitId& unit, const AbilityStateResult& result);
        AbilityStateResult HandleWeaponExecuteResult(WorldRef world, const UnitId& unit, const AbilityStateResult& result);
        AbilityStateResult HandleWeaponBackSwingResult(WorldRef world, const UnitId& unit, const AbilityStateResult& result);
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
            WorldRef world,
            const UnitId& unit,
            const Vec2& target,
            const Data::WeaponPtr& weapon,
            const Data::AttackAbilityPtr& attackAbility);

        AbilityStateResult Update(WorldRef world, const UnitId& unit);
        void Interrupt(WorldRef world, const UnitId& unit);
        void Exit(WorldRef world, const UnitId& unit);

    private:

        AbilityStateResult SetState(WorldRef world, const UnitId& unit, EActiveState state);

        AbilityStateResult EnterActiveState(WorldRef world, const UnitId& unit);
        AbilityStateResult UpdateActiveState(WorldRef world, const UnitId& unit);
        void InterruptActiveState(WorldRef world, const UnitId& unit);
        void ExitActiveState(WorldRef world, const UnitId& unit);
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
            WorldRef world,
            const UnitId& unit,
            const Vec2& target,
            const Data::WeaponPtr& weapon,
            const Data::AttackAbilityPtr& attackAbility);

        AbilityStateResult Update(WorldRef world, const UnitId& unit);
        void Interrupt(WorldRef world, const UnitId& unit);
        void Exit(WorldRef world, const UnitId& unit);
    };
}