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

TargetScanResult TargetScanner::ScanForTarget(WorldConstRef world, UnitId unit, const TargetScanArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;
    TargetScanArgs modifiedArgs = args;
    PopulateTargetScanArgs(world, unit, modifiedArgs);
    return ScanForTargetInternal(world, unit, modifiedArgs);
}

TargetScanResult TargetScanner::ScanForAbilityTarget(WorldConstRef world, UnitId unit, const TargetScanArgs& args)
{
    TargetScanArgs modifiedArgs = args;
    PopulateTargetScanArgs(world, unit, modifiedArgs);
    return ScanForAbilityTargetInternal(world, unit, modifiedArgs);
}

TargetScanResult TargetScanner::ScanForWeaponTarget(WorldConstRef world, UnitId unit, const TargetScanArgs& args)
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
        args.LdsQueryContext = LDS::FeatureLDS::StaticGetWorldQueryContext(world).get();
    }

    if (!FeatureUnit::UnitCanMove(world, unit))
    {
        SetFlagRef(args.Flags, ETargetScanFlags::UnitCannotMove);
    }
}

TargetScanResult TargetScanner::ScanForTargetInternal(WorldConstRef world, UnitId unit, const TargetScanArgs& args)
{
    // Abilities take the highest priority
    TargetScanResult result = ScanForAbilityTargetInternal(world, unit, args);
    if (result.IsValid())
    {
        return result;
    }

    return ScanForWeaponTargetInternal(world, unit, args);
}

TargetScanResult TargetScanner::ScanForAbilityTargetInternal(WorldConstRef world, UnitId unit, const TargetScanArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    std::vector<FName> abilityIds;
    if (!FeatureAbilities::GetAbilities(world, unit, abilityIds))
    {
        return {};
    }

    AbilityTargetScanArgs abilityTargetScanArgs;
    abilityTargetScanArgs.Unit = unit;
    abilityTargetScanArgs.ScanArgs = args;

    for (const FName& abilityId : abilityIds)
    {
        std::shared_ptr<IAbilityHandler> handler = FeatureAbilities::StaticFindAbilityHandler(world, abilityId);
        if (!handler)
        {
            continue;
        }

        abilityTargetScanArgs.AbilityId = abilityId;

        EntityId scanTarget = handler->ScanForTarget(world, abilityTargetScanArgs);
        if (scanTarget != EntityId::Invalid)
        {
            return {
                .Target = scanTarget,
                .TargetLocation = FeatureECS::GetWorldPosition(world, scanTarget),
                .AbilityId = abilityId,
                .WeaponId = {},
                .AcquireRequest = {}
            };
        }
    }

    return {};
}

TargetScanResult TargetScanner::ScanForWeaponTargetInternal(WorldConstRef world, UnitId unit, const TargetScanArgs& args)
{
    if (args.Level < ETargetScanLevel::Offensive)
    {
        return {};
    }

    PHX_PROFILE_ZONE_SCOPED;

    const LDS::ILDSQueryContext& lds = *args.LdsQueryContext;

    const FName& unitDataId = args.UnitDataId.Get();
    const Vec2& scanPos = args.Location.Get();

    EntityId lastScanTarget = args.LastScanTarget.GetValue(EntityId::Invalid);
    EntityId target = EntityId::Invalid;

    Data::UnitPtr unitData(unitDataId);

    std::vector<Data::WeaponPtr> weapons;
    unitData.Weapons().GetResolvedObjects(lds, weapons);

    if (weapons.empty())
    {
        return {};
    }

    bool unitCanMove = HasNoneFlags(args.Flags, ETargetScanFlags::UnitCannotMove);
    Distance minScanRange = Distance::Min;

    // Pre-compute usable weapons and their ranges
    struct WeaponScanData
    {
        FName Id;
        Distance MaxRange;
        Distance AcquireRange;
    };

    std::vector<WeaponScanData> usableWeapons;
    usableWeapons.reserve(weapons.size());

    for (const Data::WeaponPtr& weapon : weapons)
    {
        const FName& weaponId = weapon.GetObjectId();
        if (!Weapons::CanUseWeapon(world, unit, weaponId))
        {
            continue;
        }
        usableWeapons.push_back({ weaponId,
            Weapons::GetMaxRange(world, unit, weaponId),
            Weapons::GetAcquireRange(world, unit, weaponId) });
    }

    if (usableWeapons.empty())
    {
        return {};
    }

    for (const WeaponScanData& wd : usableWeapons)
    {
        Distance minHoldingRange = unitCanMove ? wd.AcquireRange : Min(wd.MaxRange, wd.AcquireRange);
        minScanRange = Max(minScanRange, minHoldingRange);
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
    rangeQueryArgs.MaxNum = 16;

    std::vector<UnitId> unitsInRange;
    FeatureUnit::QueryUnitsInRange(world, scanPos, minScanRange, unitsInRange, rangeQueryArgs);

    if (unitsInRange.empty())
    {
        return {};
    }

    TOptional<int32> bestTargetPriority;
    TOptional<FName> bestWeaponId;

    // GetAttackTargetPriority is invariant with respect to the scanning unit — hoist it out.
    int32 attackPriority = FeatureUnit::GetAttackTargetPriority(world, unit);

    for (const UnitId& candidate : unitsInRange)
    {
        if (bestTargetPriority.IsSet() && attackPriority < bestTargetPriority.Get())
        {
            continue;
        }

        for (const WeaponScanData& wd : usableWeapons)
        {
            if (!Weapons::TargetPassesFilter(world, unit, candidate, wd.Id))
                continue;

            if (!Weapons::TargetPassesAcquireFilter(world, unit, candidate, wd.Id))
                continue;

            if (Weapons::TargetIsTooClose(world, unit, candidate, wd.Id))
                continue;

            if (!Weapons::TargetIsInRange(world, unit, candidate, wd.Id) && !unitCanMove)
                continue;

            bestWeaponId = wd.Id;
            bestTargetPriority = attackPriority;
            target = candidate;
        }
    }

    if (!bestWeaponId.IsSet() || !FeatureECS::IsEntityValid(world, target))
    {
        return {};
    }

    Vec2 targetPos = FeatureECS::GetWorldPosition(world, target);

    const FName& weaponId = bestWeaponId.Get();

    if (lastScanTarget != target)
    {
        AcquireRequest request;
        request.Verb = "Attack"_n;
        request.TargetEntity = target;
        request.TargetLocation = FeatureECS::GetWorldPosition(world, target);
        request.Kind = weaponId;
        return{
            .Target = target,
            .TargetLocation = targetPos,
            .AbilityId = {},
            .WeaponId = weaponId,
            .AcquireRequest = request
        };
    }

    return {
        .Target = target,
        .TargetLocation = targetPos,
        .AbilityId = {},
        .WeaponId = weaponId,
        .AcquireRequest = {} };
}
