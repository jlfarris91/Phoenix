#pragma once

#include "PhoenixSim/ECS/EntityId.h"
#include "PhoenixSim/FixedPoint/FixedTypes.h"

#include "PhoenixRTS/Abilities/States/CommonAbilityStates.h"
#include "PhoenixRTS/Effects/EffectId.h"

namespace Phoenix::RTS
{
    struct UnitId;

    namespace Data
    {
        struct WeaponPtr;
    }
    
    struct WeaponTargetEntityState : TargetEntityState
    {
        FName WeaponId;

        AbilityStateResult Update(WorldRef world, const UnitId& unit) const;
        void KeepPreSwingHot(WorldRef world, const UnitId& unit) const;
    };

    struct WeaponTargetLocationState : TargetLocationState
    {
        FName WeaponId;

        AbilityStateResult Update(WorldRef world, const UnitId& unit) const;
        void KeepPreSwingHot(WorldRef world, const UnitId& unit) const;
    };

    struct WeaponPreSwingState : WeaponTargetEntityState
    {
        Time CompletedTime;

        AbilityStateResult Enter(
            WorldRef world,
            const UnitId& unit,
            const Data::WeaponPtr& weapon,
            const ECS::EntityId& target);

        AbilityStateResult Update(WorldRef world, const UnitId& unit) const;
        void Interrupt(WorldRef world, const UnitId& unit) const;
        void Exit(WorldRef world, const UnitId& unit) const;
    };

    struct WeaponCooldownState : WeaponTargetEntityState
    {
        AbilityStateResult Enter(
            WorldRef world,
            const UnitId& unit,
            const Data::WeaponPtr& weapon,
            const ECS::EntityId& target);

        AbilityStateResult Update(WorldRef world, const UnitId& unit) const;
        void Interrupt(WorldRef world, const UnitId& unit) const;
        void Exit(WorldRef world, const UnitId& unit) const;
    };

    struct WeaponSwingState : WeaponTargetEntityState
    {
        Time CompletedTime;

        AbilityStateResult Enter(
            WorldRef world,
            const UnitId& unit,
            const Data::WeaponPtr& weapon,
            const ECS::EntityId& target);

        AbilityStateResult Update(WorldRef world, const UnitId& unit) const;
        void Interrupt(WorldRef world, const UnitId& unit) const;
        void Exit(WorldRef world, const UnitId& unit) const;
    };

    struct WeaponExecuteState : WeaponTargetEntityState
    {
        EffectScopeId EffectScope;
        bool Interrupted = false;

        AbilityStateResult Enter(
            WorldRef world,
            const UnitId& unit,
            const Data::WeaponPtr& weapon,
            const ECS::EntityId& target);

        AbilityStateResult Update(WorldRef world, const UnitId& unit) const;
        void Interrupt(WorldRef world, const UnitId& unit);
        void Exit(WorldRef world, const UnitId& unit) const;
    };

    struct WeaponBackSwingState : WeaponTargetEntityState
    {
        Time CompletedTime;

        AbilityStateResult Enter(
            WorldRef world,
            const UnitId& unit,
            const Data::WeaponPtr& weapon,
            const ECS::EntityId& target);

        AbilityStateResult Update(WorldRef world, const UnitId& unit) const;
        void Interrupt(WorldRef world, const UnitId& unit) const;
        void Exit(WorldRef world, const UnitId& unit) const;
    };
}
