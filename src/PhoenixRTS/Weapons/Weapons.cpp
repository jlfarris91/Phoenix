#include "PhoenixRTS/Weapons/Weapons.h"

#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/LDS/FeatureLDS.h"

#include "PhoenixRTS/Data/DataUnit.h"
#include "PhoenixRTS/Data/DataWeapon.h"
#include "PhoenixRTS/Data/DataWeaponAmmo.h"
#include "PhoenixRTS/Effects/FeatureEffects.h"
#include "PhoenixRTS/TargetFiltering/TargetFiltering.h"
#include "PhoenixRTS/Units/FeatureUnit.h"
#include "PhoenixRTS/Units/UnitId.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

bool Weapons::CanUseWeapon(WorldConstRef world, const UnitId& unit, const FName& weaponId)
{
    const ILDSQueryContext& lds = *FeatureLDS::StaticGetWorldQueryContext(world);

    Data::WeaponPtr weaponPtr(weaponId);
    if (!weaponPtr.Exists(lds))
    {
        return false;
    }

    // TODO (jfarris): validate whether the weapon can be used
    return true;
}

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
    const ILDSQueryContext& lds = *FeatureLDS::StaticGetWorldQueryContext(world);
    Data::WeaponAmmoPtr weaponAmmoPtr(ammoId);
    return weaponAmmoPtr.Capacity().GetValue(lds);
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
    const ILDSQueryContext& lds = *FeatureLDS::StaticGetWorldQueryContext(world);
    Data::WeaponPtr weapon(weaponId);
    return weapon.SwingTime().GetValue(lds) - weapon.PreSwingCooldown().GetValue(lds);
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
    const ILDSQueryContext& lds = *FeatureLDS::StaticGetWorldQueryContext(world);
    Data::WeaponPtr weapon(weaponId);
    FName ammoId = weapon.Ammo().GetReferenceId(lds);
    return GetAmmoRemaining(world, unit, ammoId) > 0 && !IsCoolingDown(world, unit, weaponId);
}

Distance Weapons::GetMinRange(WorldConstRef world, const UnitId& unit, const FName& weaponId)
{
    const ILDSQueryContext& lds = *FeatureLDS::StaticGetWorldQueryContext(world);
    Data::WeaponPtr weapon(weaponId);
    return weapon.RangeMin().GetValue(lds);
}

Distance Weapons::GetMaxRange(WorldConstRef world, const UnitId& unit, const FName& weaponId)
{
    const ILDSQueryContext& lds = *FeatureLDS::StaticGetWorldQueryContext(world);
    Data::WeaponPtr weapon(weaponId);
    return weapon.RangeMax().GetValue(lds);
}

Distance Weapons::GetAcquireRange(WorldConstRef world, const UnitId& unit, const FName& weaponId)
{
    const ILDSQueryContext& lds = *FeatureLDS::StaticGetWorldQueryContext(world);
    Data::WeaponPtr weapon(weaponId);
    return weapon.RangeAcquire().GetValue(lds);
}

bool Weapons::TargetIsTooClose(WorldConstRef world, const UnitId& unit, const EntityId& target, const FName& weaponId)
{
    const ILDSQueryContext& lds = *FeatureLDS::StaticGetWorldQueryContext(world);
    Data::WeaponPtr weapon(weaponId);

    Distance rangeMin = weapon.RangeMin().GetValue(lds);
    return rangeMin > 0 && FeatureECS::IsInRange(world, unit, target, rangeMin);
}

bool Weapons::TargetIsTooClose(WorldConstRef world, const UnitId& unit, const Vec2& target, const FName& weaponId)
{
    const ILDSQueryContext& lds = *FeatureLDS::StaticGetWorldQueryContext(world);
    Data::WeaponPtr weapon(weaponId);

    Distance rangeMin = weapon.RangeMin().GetValue(lds);
    return rangeMin > 0 && FeatureECS::IsInRange(world, unit, target, rangeMin);
}

bool Weapons::TargetIsInRange(
    WorldConstRef world,
    const UnitId& unit,
    const EntityId& target,
    const FName& weaponId,
    Distance bonusRange)
{
    const ILDSQueryContext& lds = *FeatureLDS::StaticGetWorldQueryContext(world);
    Data::WeaponPtr weapon(weaponId);

    Distance range = weapon.RangeMax().GetValue(lds);
    Distance rangeSlop = weapon.RangeSlop().GetValue(lds);

    Data::EWeaponFlags flags = weapon.Flags().GetValue(lds);
    if (HasAnyFlags(flags, Data::EWeaponFlags::Melee))
    {
        bonusRange = 0;
    }
    
    return FeatureECS::IsInRange(world, unit, target, range + rangeSlop + bonusRange);
}

bool Weapons::TargetIsInRange(
    WorldConstRef world,
    const UnitId& unit,
    const Vec2& target,
    const FName& weaponId,
    Distance bonusRange)
{
    const ILDSQueryContext& lds = *FeatureLDS::StaticGetWorldQueryContext(world);
    Data::WeaponPtr weapon(weaponId);

    Distance rangeMax = weapon.RangeMax().GetValue(lds);
    Distance rangeSlop = weapon.RangeSlop().GetValue(lds);
    return FeatureECS::IsInRange(world, unit, target, rangeMax + rangeSlop + bonusRange);
}

Angle Weapons::GetWeaponArcMin(WorldConstRef world, const UnitId& unit, const FName& weaponId)
{
    const ILDSQueryContext& lds = *FeatureLDS::StaticGetWorldQueryContext(world);
    Data::WeaponPtr weapon(weaponId);
    return weapon.FacingArcMin().GetValue(lds);
}

Angle Weapons::GetWeaponArcSlop(WorldConstRef world, const UnitId& unit, const FName& weaponId)
{
    const ILDSQueryContext& lds = *FeatureLDS::StaticGetWorldQueryContext(world);
    Data::WeaponPtr weapon(weaponId);
    return weapon.FacingArcSlop().GetValue(lds);
}

Angle Weapons::GetWeaponArcPlusSlop(WorldConstRef world, const UnitId& unit, const FName& weaponId)
{
    const ILDSQueryContext& lds = *FeatureLDS::StaticGetWorldQueryContext(world);
    Data::WeaponPtr weapon(weaponId);
    return weapon.FacingArcMin().GetValue(lds) + weapon.FacingArcSlop().GetValue(lds);
}

bool Weapons::IsFacingTarget(
    WorldConstRef world,
    const UnitId& unit,
    const EntityId& target,
    const FName& weaponId)
{
    Angle arc = GetWeaponArcMin(world, unit, weaponId);
    return FeatureECS::IsFacing(world, unit, target, arc);
}

bool Weapons::IsFacingTarget(
    WorldConstRef world,
    const UnitId& unit,
    const Vec2& target,
    const FName& weaponId)
{
    Angle arc = GetWeaponArcMin(world, unit, weaponId);
    return FeatureECS::IsFacing(world, unit, target, arc);
}

bool Weapons::TargetPassesFilter(
    WorldConstRef world,
    const UnitId& source,
    const EntityId& target,
    const FName& weaponId)
{
    const ILDSQueryContext& lds = *FeatureLDS::StaticGetWorldQueryContext(world);

    Data::WeaponPtr weapon(weaponId);
    if (!weapon.Exists(lds))
    {
        return false;
    }

    Data::TargetFilter targetFilter = weapon.TargetFilter().ReadObject(lds);
    return TargetFiltering::PassesTargetFilter(world, targetFilter, source, target);
}

bool Weapons::TargetPassesAcquireFilter(
    WorldConstRef world,
    const UnitId& source,
    const EntityId& target,
    const FName& weaponId)
{
    const ILDSQueryContext& lds = *FeatureLDS::StaticGetWorldQueryContext(world);

    Data::WeaponPtr weapon(weaponId);
    if (!weapon.Exists(lds))
    {
        return false;
    }

    Data::TargetFilter acquireFilter = weapon.AcquireFilter().ReadObject(lds);
    return TargetFiltering::PassesTargetFilter(world, acquireFilter, source, target);
}

RTS::Data::WeaponPtr Weapons::FindBestEnabledWeapon(
    WorldConstRef world,
    const UnitId& unit,
    const EntityId& target,
    bool canUnitMove)
{
    uint32 index;
    return FindBestEnabledWeapon(world, unit, target, canUnitMove, index);
}

RTS::Data::WeaponPtr Weapons::FindBestEnabledWeapon(
    WorldConstRef world,
    const UnitId& unit,
    const EntityId& target,
    bool canUnitMove,
    uint32& outIndex)
{
    const ILDSQueryContext& lds = *FeatureLDS::StaticGetWorldQueryContext(world);

    FName unitDataId = FeatureUnit::GetUnitDataId(world, unit);
    Data::UnitPtr unitData(unitDataId);

    if (!unitData.Exists(lds) ||
        !FeatureECS::IsEntityValid(world, target) ||
        FeatureUnit::UnitIsHidden(world, UnitId(target)) ||
        FeatureUnit::UnitIsCargo(world, UnitId(target)))
    {
        return {};
    }

    Data::WeaponPtr bestWeaponData;

    unitData.Weapons().ForEachResolvedItemObject(lds, [&](uint32 index, const Data::WeaponPtr& weaponData)
    {
        FName weaponObjectId = weaponData.GetObjectId();

        if (!CanUseWeapon(world, unit, weaponObjectId))
        {
            return false;
        }

        if (!TargetPassesFilter(world, unit, target, weaponObjectId))
        {
            return false;
        }

        if (TargetIsTooClose(world, unit, target, weaponObjectId))
        {
            return false;
        }

        Distance weaponRange = GetMaxRange(world, unit, weaponObjectId);
        if (FeatureECS::IsInRange(world, unit, target, weaponRange))
        {
            outIndex = index;
            bestWeaponData = weaponData;
            return true;
        }

        if (!bestWeaponData.IsValid() && canUnitMove)
        {
            outIndex = index;
            bestWeaponData = weaponData;
        }

        return false;
    });

    return bestWeaponData;
}

RTS::Data::WeaponPtr Weapons::FindBestEnabledWeapon(
    WorldConstRef world,
    const UnitId& unit,
    const Vec2& target,
    bool canUnitMove)
{
    uint32 index;
    return FindBestEnabledWeapon(world, unit, target, canUnitMove, index);
}

RTS::Data::WeaponPtr Weapons::FindBestEnabledWeapon(
    WorldConstRef world,
    const UnitId& unit,
    const Vec2& target,
    bool canUnitMove,
    uint32& outIndex)
{
    const ILDSQueryContext& lds = *FeatureLDS::StaticGetWorldQueryContext(world);

    FName unitDataId = FeatureUnit::GetUnitDataId(world, unit);
    Data::UnitPtr unitData(unitDataId);

    Data::WeaponPtr bestWeaponData;

    unitData.Weapons().ForEachResolvedItemObject(lds, [&](uint32 index, const Data::WeaponPtr& weaponData)
    {
        FName weaponObjectId = weaponData.GetObjectId();

        if (!CanUseWeapon(world, unit, weaponObjectId))
        {
            return false;
        }

        if (TargetIsTooClose(world, unit, target, weaponObjectId))
        {
            return false;
        }

        Distance weaponRange = GetMaxRange(world, unit, weaponObjectId);
        if (FeatureECS::IsInRange(world, unit, target, weaponRange))
        {
            outIndex = index;
            bestWeaponData = weaponData;
            return true;
        }

        if (!bestWeaponData.IsValid() && canUnitMove)
        {
            outIndex = index;
            bestWeaponData = weaponData;
        }

        return false;
    });

    return bestWeaponData;
}
