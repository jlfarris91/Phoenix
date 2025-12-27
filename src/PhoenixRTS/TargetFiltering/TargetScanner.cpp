#include "TargetScanner.h"

#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/LDS/FeatureLDS.h"

#include "PhoenixRTS/Units/FeatureUnit.h"
#include "PhoenixRTS/Abilities/FeatureAbilities.h"
#include "PhoenixRTS/Data/DataUnit.h"
#include "PhoenixRTS/Orders/FeatureOrders.h"
#include "PhoenixRTS/Weapons/Weapons.h"
#include "PhoenixSteering/FeatureSteering.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::Steering;
using namespace Phoenix::RTS;

bool TargetScanResult::IsValid() const
{
    return Target != EntityId::Invalid;
}

TargetScanResult TargetScanner::ScanForTarget(WorldRef world, UnitId unit, const TargetScanArgs& args)
{
    TargetScanArgs modifiedArgs = args;
    PopulateTargetScanArgs(world, unit, modifiedArgs);
    return ScanForTargetInternal(world, unit, modifiedArgs);
}

TargetScanResult TargetScanner::ScanForAbilityTarget(WorldRef world, UnitId unit, const TargetScanArgs& args)
{
    TargetScanArgs modifiedArgs = args;
    PopulateTargetScanArgs(world, unit, modifiedArgs);
    return ScanForAbilityTargetInternal(world, unit, modifiedArgs);
}

TargetScanResult TargetScanner::ScanForWeaponTarget(WorldRef world, UnitId unit, const TargetScanArgs& args)
{
    TargetScanArgs modifiedArgs = args;
    PopulateTargetScanArgs(world, unit, modifiedArgs);
    return ScanForWeaponTargetInternal(world, unit, modifiedArgs);
}

void TargetScanner::PopulateTargetScanArgs(WorldConstRef world, UnitId unit, TargetScanArgs& args)
{
    if (!args.UnitDataId.IsSet())
    {
        args.UnitDataId = FeatureUnit::GetUnitDataId(world, unit);
    }

    if (!args.Location.IsSet())
    {
        args.Location = FeatureECS::GetWorldPosition(world, unit);
    }

    if (!args.LastScanTarget.IsSet())
    {
        TOptional<EntityId> currTarget = FeatureOrders::GetHeadOrderTargetEntity(world, unit);
        if (currTarget.IsSet())
        {
            args.LastScanTarget = currTarget.Get();
        }
    }

    if (!args.LdsQueryContext)
    {
        args.LdsQueryContext = LDS::FeatureLDS::StaticGetWorldQueryContext(world);
    }

    if (!FeatureUnit::UnitCanMove(world, unit))
    {
        SetFlagRef(args.Flags, ETargetScanFlags::UnitCannotMove);
    }
}

TargetScanResult TargetScanner::ScanForTargetInternal(WorldRef world, UnitId unit, const TargetScanArgs& args)
{
    // Abilities take the highest priority
    TargetScanResult result = ScanForAbilityTargetInternal(world, unit, args);
    if (result.IsValid())
    {
        return result;
    }

    return ScanForWeaponTargetInternal(world, unit, args);
}

TargetScanResult TargetScanner::ScanForAbilityTargetInternal(WorldRef world, UnitId unit, const TargetScanArgs& args)
{
    TArray2<FName> abilityIds;
    if (!FeatureAbilities::GetAbilities(world, unit, abilityIds))
    {
        return {};
    }

    AbilityTargetScanArgs abilityTargetScanArgs;
    abilityTargetScanArgs.Unit = unit;
    abilityTargetScanArgs.ScanArgs = args;

    for (const FName& abilityId : abilityIds)
    {
        TSharedPtr<IAbilityHandler> handler = FeatureAbilities::StaticFindAbilityHandler(world, abilityId);
        if (!handler)
        {
            continue;
        }

        abilityTargetScanArgs.AbilityId = abilityId;

        EntityId scanTarget = handler->ScanForTarget(world, abilityTargetScanArgs);
        if (scanTarget != EntityId::Invalid)
        {
            return { scanTarget, abilityId, {} };
        }
    }

    return {};
}

TargetScanResult TargetScanner::ScanForWeaponTargetInternal(WorldRef world, UnitId unit, const TargetScanArgs& args)
{
    if (args.Level < ETargetScanLevel::Offensive)
    {
        return {};
    }
    
    const LDS::ILDSQueryContext& lds = *args.LdsQueryContext;

    const FName& unitDataId = args.UnitDataId.Get();
    const Vec2& scanPos = args.Location.Get();

    EntityId lastScanTarget = args.LastScanTarget.GetValue(EntityId::Invalid);
    EntityId target = EntityId::Invalid;

    Data::UnitPtr unitData(unitDataId);

    TArray2<Data::WeaponPtr> weapons;
    unitData.Weapons().GetResolvedObjects(lds, weapons);

    if (weapons.IsEmpty())
    {
        return {};
    }

    bool unitCanMove = HasNoneFlags(args.Flags, ETargetScanFlags::UnitCannotMove);
    Distance minScanRange = Distance::Min;

    for (const Data::WeaponPtr& weapon : weapons)
    {
        if (Weapons::CanUseWeapon(world, unit, weapon.GetObjectId()))
        {
            Distance maxRange = Weapons::GetMaxRange(world, unit, weapon.GetObjectId());
            Distance acquireRange = Weapons::GetAcquireRange(world, unit, weapon.GetObjectId());
            Distance minHoldingRange = unitCanMove ? acquireRange : Min(maxRange, acquireRange);
            minScanRange = Max(minScanRange, minHoldingRange);
        }
    }

    if (minScanRange <= 0.0)
    {
        return {};
    }

    Distance unitRadius = FeatureSteering::GetEntityOuterRadius(world, unit);
    minScanRange += unitRadius;

    UnitRangeQueryArgs rangeQueryArgs;
    rangeQueryArgs.Exclude = { unit };
    rangeQueryArgs.Flags = EUnitQueryFlags::Alive;
    rangeQueryArgs.TeamMask = Teams::EnemiesOf(world, FeatureUnit::GetOwningTeam(world, unit));

    TArray2<UnitId> unitsInRange;
    FeatureUnit::QueryUnitsInRange(world, scanPos, minScanRange, unitsInRange, rangeQueryArgs);

    if (unitsInRange.IsEmpty())
    {
        return {};
    }

    TOptional<int32> bestTargetPriority;
    TOptional<uint32> bestWeaponIndex;

    for (const UnitId& candidate : unitsInRange)
    {
        int32 attackPriority = FeatureUnit::GetAttackTargetPriority(world, unit);

        if (bestTargetPriority.IsSet() && attackPriority < bestTargetPriority.Get())
        {
            continue;
        }

        uint32 weaponIndex = 0;
        for (const Data::WeaponPtr& weapon : weapons)
        {
            const FName& weaponId = weapon.GetObjectId();

            if (!Weapons::CanUseWeapon(world, unit, weaponId))
            {
                continue;
            }

            if (!Weapons::TargetPassesFilter(world, unit, candidate, weaponId))
            {
                continue;
            }

            if (!Weapons::TargetPassesAcquireFilter(world, unit, candidate, weaponId))
            {
                continue;
            }

            if (Weapons::TargetIsTooClose(world, unit, candidate, weaponId))
            {
                continue;
            }

            bestWeaponIndex = weaponIndex++;
            bestTargetPriority = attackPriority;
            target = candidate;
        }
    }

    if (!bestWeaponIndex.IsSet() || !FeatureECS::IsEntityValid(world, target))
    {
        return {};
    }

    FName bestWeaponId = weapons[bestWeaponIndex.Get()].GetObjectId();

    if (lastScanTarget != target && HasAnyFlags(args.Flags, ETargetScanFlags::AutoAcquire))
    {
        AcquireRequest request;
        request.Verb = "Attack"_n;
        request.TargetEntity = target;
        request.TargetLocation = FeatureECS::GetWorldPosition(world, target);
        request.Kind = bestWeaponId;
        FeatureOrders::StaticRequestAcquireOrder(world, unit, request);
    }

    return { target, {}, bestWeaponId };
}
