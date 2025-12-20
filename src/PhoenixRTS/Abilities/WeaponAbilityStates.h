#pragma once

#include "PhoenixSim/ECS/EntityId.h"
#include "PhoenixSim/FixedPoint/FixedTypes.h"

#include "PhoenixRTS/Abilities/AbilityStates.h"
#include "PhoenixRTS/Data/DataWeapon.h"
#include "PhoenixRTS/Effects/EffectId.h"

namespace Phoenix::RTS
{
    struct WeaponStateBase
    {
        FName WeaponId;
        ECS::EntityId Target;

        AbilityStateResult OnUpdate(WorldRef world, const UnitId& unit) const;
        void KeepPreSwingHot(WorldRef world, const UnitId& unit) const;
    };

    struct WeaponPreSwingState : WeaponStateBase
    {
        Time CompletedTime;

        AbilityStateResult OnEnter(
            WorldRef world,
            const UnitId& unit,
            const Data::WeaponPtr& weapon,
            const ECS::EntityId& target);

        AbilityStateResult OnUpdate(WorldRef world, const UnitId& unit) const;
        void OnInterrupt(WorldRef world, const UnitId& unit) const;
    };

    struct WeaponCooldownState : WeaponStateBase
    {
        AbilityStateResult OnEnter(
            WorldRef world,
            const UnitId& unit,
            const Data::WeaponPtr& weapon,
            const ECS::EntityId& target);

        AbilityStateResult OnUpdate(WorldRef world, const UnitId& unit) const;
        void OnInterrupt(WorldRef world, const UnitId& unit) const;
    };

    struct WeaponSwingState : WeaponStateBase
    {
        Time CompletedTime;

        AbilityStateResult OnEnter(
            WorldRef world,
            const UnitId& unit,
            const Data::WeaponPtr& weapon,
            const ECS::EntityId& target);

        AbilityStateResult OnUpdate(WorldRef world, const UnitId& unit) const;
        void OnInterrupt(WorldRef world, const UnitId& unit) const;
    };

    struct WeaponExecuteState : WeaponStateBase
    {
        EffectScopeId EffectScope;
        bool Interrupted = false;

        AbilityStateResult OnEnter(
            WorldRef world,
            const UnitId& unit,
            const Data::WeaponPtr& weapon,
            const ECS::EntityId& target);

        AbilityStateResult OnUpdate(WorldRef world, const UnitId& unit) const;
        void OnInterrupt(WorldRef world, const UnitId& unit);
        void OnExit(WorldRef world, const UnitId& unit) const;
    };

    struct WeaponBackSwingState : WeaponStateBase
    {
        Time CompletedTime;

        AbilityStateResult OnEnter(
            WorldRef world,
            const UnitId& unit,
            const Data::WeaponPtr& weapon,
            const ECS::EntityId& target);

        AbilityStateResult OnUpdate(WorldRef world, const UnitId& unit) const;
        void OnInterrupt(WorldRef world, const UnitId& unit) const;
        void OnExit(WorldRef world, const UnitId& unit) const;
    };
}
