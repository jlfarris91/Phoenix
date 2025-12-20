#include "PhoenixRTS/Weapons/Weapons.h"

#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/LDS/FeatureLDS.h"

#include "PhoenixRTS/Data/DataWeapon.h"
#include "PhoenixRTS/Data/DataWeaponAmmo.h"
#include "PhoenixRTS/Units/UnitId.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

uint32 Weapons::GetAmmoSpent(WorldConstRef world, const UnitId& unit, const FName& ammoId)
{
    return FeatureECS::GetBlackboardValue<uint32>(world, unit, ammoId.Append("ammo_spent"));
}

bool Weapons::SetAmmoSpent(WorldRef world, const UnitId& unit, const FName& ammoId, uint32 spent)
{
    return FeatureECS::SetBlackboardValue<uint32>(world, unit, ammoId.Append("ammo_spent"), std::max(spent, 0u));
}

uint32 Weapons::ExpendAmmo(WorldRef world, const UnitId& unit, const FName& ammoId, int32 amount)
{
    uint32 ammoCapacity = GetAmmoCapacity(world, ammoId);
    int32 newSpent = static_cast<int32>(GetAmmoSpent(world, unit, ammoId)) + amount;
    newSpent = Clamp<int32>(newSpent, 0, ammoCapacity);
    SetAmmoSpent(world, unit, ammoId, newSpent);
    return newSpent;
}

uint32 Weapons::GetAmmoRemaining(WorldConstRef world, const UnitId& unit, const FName& ammoId)
{
    return GetAmmoCapacity(world, ammoId) - GetAmmoSpent(world, unit, ammoId);
}

uint32 Weapons::GetAmmoCapacity(WorldConstRef world, const FName& ammoId)
{
    const LDS::ILDSQueryContext& queryContext = *LDS::FeatureLDS::StaticGetWorldQueryContext(world);
    Data::WeaponAmmoPtr weaponAmmoPtr(ammoId);
    return weaponAmmoPtr.Capacity().GetValue(queryContext);
}

Time Weapons::GetCooldownEndTime(WorldConstRef world, const UnitId& unit, const FName& weaponId)
{
    return FeatureECS::GetBlackboardValue<Time>(world, unit, weaponId.Append("cooldown_end_time"));
}

bool Weapons::SetCooldownEndTime(WorldRef world, const UnitId& unit, const FName& weaponId, Time time)
{
    return FeatureECS::SetBlackboardValue<Time>(world, unit, weaponId.Append("cooldown_end_time"), time);
}

Time Weapons::GetCooldownDuration(WorldConstRef world, const UnitId& unit, const FName& weaponId)
{
    const LDS::ILDSQueryContext& queryContext = *LDS::FeatureLDS::StaticGetWorldQueryContext(world);
    Data::WeaponPtr weapon(weaponId);
    return weapon.SwingTime().GetValue(queryContext) - weapon.PreSwingCooldown().GetValue(queryContext);
}

bool Weapons::StartCooldown(WorldRef world, const UnitId& unit, const FName& weaponId)
{
    Time cooldownDuration = GetCooldownDuration(world, unit, weaponId);
    Time cooldownEndTime = world.GetSimTime() + cooldownDuration;
    return SetCooldownEndTime(world, unit, weaponId, cooldownEndTime);
}

bool Weapons::IsCoolingDown(WorldConstRef world, const UnitId& unit, const FName& weaponId)
{
    return world.GetSimTime() < GetCooldownEndTime(world, unit, weaponId);
}

bool Weapons::HasAmmoAndNotOnCooldown(WorldConstRef world, const UnitId& unit, const FName& weaponId)
{
    const LDS::ILDSQueryContext& queryContext = *LDS::FeatureLDS::StaticGetWorldQueryContext(world);
    Data::WeaponPtr weapon(weaponId);
    FName ammoId = weapon.Ammo().GetReferenceId(queryContext);
    return GetAmmoRemaining(world, unit, ammoId) > 0 && !IsCoolingDown(world, unit, weaponId);
}

bool Weapons::TargetIsTooClose(WorldConstRef world, const UnitId& unit, const EntityId& target, const FName& weaponId)
{
    const LDS::ILDSQueryContext& queryContext = *LDS::FeatureLDS::StaticGetWorldQueryContext(world);
    Data::WeaponPtr weapon(weaponId);

    Distance rangeMin = weapon.RangeMin().GetValue(queryContext);
    return rangeMin > 0 && FeatureECS::IsInRange(world, unit, target, rangeMin);
}

bool Weapons::TargetIsTooClose(WorldConstRef world, const UnitId& unit, const Vec2& target, const FName& weaponId)
{
    const LDS::ILDSQueryContext& queryContext = *LDS::FeatureLDS::StaticGetWorldQueryContext(world);
    Data::WeaponPtr weapon(weaponId);

    Distance rangeMin = weapon.RangeMin().GetValue(queryContext);
    return rangeMin > 0 && FeatureECS::IsInRange(world, unit, target, rangeMin);
}

bool Weapons::TargetIsInRange(
    WorldConstRef world,
    const UnitId& unit,
    const EntityId& target,
    const FName& weaponId,
    Distance bonusRange)
{
    const LDS::ILDSQueryContext& queryContext = *LDS::FeatureLDS::StaticGetWorldQueryContext(world);
    Data::WeaponPtr weapon(weaponId);

    Distance rangeMax = weapon.RangeMax().GetValue(queryContext);
    Distance rangeSlop = weapon.RangeSlop().GetValue(queryContext);
    return FeatureECS::IsInRange(world, unit, target, rangeMax + rangeSlop + bonusRange);
}

bool Weapons::TargetIsInRange(
    WorldConstRef world,
    const UnitId& unit,
    const Vec2& target,
    const FName& weaponId,
    Distance bonusRange)
{
    const LDS::ILDSQueryContext& queryContext = *LDS::FeatureLDS::StaticGetWorldQueryContext(world);
    Data::WeaponPtr weapon(weaponId);

    Distance rangeMax = weapon.RangeMax().GetValue(queryContext);
    Distance rangeSlop = weapon.RangeSlop().GetValue(queryContext);
    return FeatureECS::IsInRange(world, unit, target, rangeMax + rangeSlop + bonusRange);
}

bool Weapons::TargetPassesFilter(
    WorldConstRef world,
    const UnitId& unit,
    const EntityId& target,
    const FName& weaponId)
{
    // TODO (jfarris): implement target filtering
    return true;
}
