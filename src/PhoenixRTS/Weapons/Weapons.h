#pragma once

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Worlds.h"
#include "PhoenixSim/FixedPoint/FixedVector.h"

#include "PhoenixRTS/Abilities/WeaponAbilityStates.h"

namespace Phoenix::ECS
{
    struct EntityId;
}

namespace Phoenix::RTS
{
    struct UnitId;

    struct PHOENIX_RTS_API Weapons
    {
        // Gets the amount of ammo spent by a unit.
        static uint32 GetAmmoSpent(WorldConstRef world, const UnitId& unit, const FName& ammoId);

        // Sets the amount of ammo spent by a unit.
        static bool SetAmmoSpent(WorldRef world, const UnitId& unit, const FName& ammoId, uint32 spent);

        // Reduces the ammo remaining by an amount.
        static uint32 ExpendAmmo(WorldRef world, const UnitId& unit, const FName& ammoId, int32 amount = 1);

        // Gets the amount of ammo remaining for a unit.
        static uint32 GetAmmoRemaining(WorldConstRef world, const UnitId& unit, const FName& ammoId);

        // Gets the capacity of a weapons magazine.
        static uint32 GetAmmoCapacity(WorldConstRef world, const FName& ammoId);

        // Gets the time that the weapon will end its cooldown.
        static Time GetCooldownEndTime(WorldConstRef world, const UnitId& unit, const FName& weaponId);

        // Sets the time that the weapon will end its cooldown.
        static bool SetCooldownEndTime(WorldRef world, const UnitId& unit, const FName& weaponId, Time time);

        static Time GetCooldownDuration(WorldConstRef world, const UnitId& unit, const FName& weaponId);

        // Starts the cooldown for a weapon.
        static bool StartCooldown(WorldRef world, const UnitId& unit, const FName& weaponId);

        // Returns true if the weapon is currently cooling down.
        static bool IsCoolingDown(WorldConstRef world, const UnitId& unit, const FName& weaponId);

        // Returns true if the weapon has ammo and is not currently cooling down.
        static bool HasAmmoAndNotOnCooldown(WorldConstRef world, const UnitId& unit, const FName& weaponId);

        // Returns true if the target is within the minimum range of the weapon.
        static bool TargetIsTooClose(WorldConstRef world, const UnitId& unit, const ECS::EntityId& target, const FName& weaponId);

        // Returns true if the target is within the minimum range of the weapon.
        static bool TargetIsTooClose(WorldConstRef world, const UnitId& unit, const Vec2& target, const FName& weaponId);

        // Returns true if the target is outside the maximum range of the weapon.
        static bool TargetIsInRange(
            WorldConstRef world,
            const UnitId& unit,
            const ECS::EntityId& target,
            const FName& weaponId,
            Distance bonusRange = 0);

        // Returns true if the target is outside the maximum range of the weapon.
        static bool TargetIsInRange(
            WorldConstRef world,
            const UnitId& unit,
            const Vec2& target,
            const FName& weaponId,
            Distance bonusRange = 0);

        // Returns true if the target passes the weapons target filter.
        static bool TargetPassesFilter(
            WorldConstRef world,
            const UnitId& unit,
            const ECS::EntityId& target,
            const FName& weaponId);
    };
}
