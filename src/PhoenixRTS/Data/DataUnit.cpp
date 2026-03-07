
#include "PhoenixRTS/Data/DataUnit.h"

using namespace Phoenix::RTS::Data;

bool Unit::Read(const LDS::LDSReadObjectArgs& args, Unit& outItem)
{
    const LDS::ILDSQueryContext& queryContext = args.GetQueryContext();

    bool success = true;

    UnitPtr dataPtr = args.CreatePtr<UnitPtr>();
    success = dataPtr.Actor().TryResolveObject(queryContext, outItem.Actor) && success;
    success = dataPtr.Armor().TryReadObject(queryContext, outItem.Armor) && success;
    success = dataPtr.Buffs().GetResolvedObjects(queryContext, outItem.Buffs) && success;
    success = dataPtr.BuildStats().TryReadObject(queryContext, outItem.BuildStats) && success;
    success = dataPtr.CargoStats().TryReadObject(queryContext, outItem.CargoStats) && success;
    success = dataPtr.CollisionFlags().TryGetValue(queryContext, outItem.CollisionFlags) && success;
    success = dataPtr.Commands().ReadObjects(queryContext, outItem.Commands) && success;
    success = dataPtr.Components().ReadObjects(queryContext, outItem.Components) && success;
    success = dataPtr.DeathStats().TryReadObject(queryContext, outItem.DeathStats) && success;
    success = dataPtr.Effects().TryReadObject(queryContext, outItem.Effects) && success;
    success = dataPtr.Faction().TryResolveObject(queryContext, outItem.Faction) && success;
    success = dataPtr.Flags().TryReadObject(queryContext, outItem.Flags) && success;
    success = dataPtr.Fog().TryReadObject(queryContext, outItem.Fog) && success;
    success = dataPtr.Info().TryReadObject(queryContext, outItem.Info) && success;
    success = dataPtr.Movement().TryReadObject(queryContext, outItem.Movement) && success;
    success = dataPtr.Placement().TryReadObject(queryContext, outItem.Placement) && success;
    success = dataPtr.Tags().GetResolvedObjects(queryContext, outItem.Tags) && success;
    success = dataPtr.SelectionCircleScale().TryGetValue(queryContext, outItem.SelectionCircleScale) && success;
    success = dataPtr.Supply().TryReadObject(queryContext, outItem.Supply) && success;
    success = dataPtr.Vision().TryReadObject(queryContext, outItem.Vision) && success;
    success = dataPtr.Weapons().GetResolvedObjects(queryContext, outItem.Weapons) && success;

    return success;
}

UnitPtr::UnitPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

UnitActorRefPtr UnitPtr::Actor() const
{
    return ObjectRef<UnitActorRefPtr>("actor");
}

UnitArmorPtr UnitPtr::Armor() const
{
    return Object<UnitArmorPtr>("armor");
}

BuffRefArrayPtr UnitPtr::Buffs() const
{
    return ObjectRefArray<BuffRefArrayPtr>("buffs");
}

UnitBuildPtr UnitPtr::BuildStats() const
{
    return Object<UnitBuildPtr>("build_stats");
}

UnitCargoPtr UnitPtr::CargoStats() const
{
    return Object<UnitCargoPtr>("cargo");
}

Phoenix::LDS::TLDSEnumFlagsPtr<ECollisionFlags> UnitPtr::CollisionFlags() const
{
    return EnumFlags<ECollisionFlags>("collision");
}

CommandArrayPtr UnitPtr::Commands() const
{
    return ObjectArray<CommandArrayPtr>("commands");
}

ComponentArrayPtr UnitPtr::Components() const
{
    return ObjectArray<ComponentArrayPtr>("components");
}

UnitDeathPtr UnitPtr::DeathStats() const
{
    return Object<UnitDeathPtr>("death_stats");
}

UnitEffectsPtr UnitPtr::Effects() const
{
    return Object<UnitEffectsPtr>("death_stats");
}

FactionRefPtr UnitPtr::Faction() const
{
    return ObjectRef<FactionRefPtr>("faction");
}

UnitFlagsPtr UnitPtr::Flags() const
{
    return Object<UnitFlagsPtr>("flags");
}

FogVisibilityPtr UnitPtr::Fog() const
{
    return Object<FogVisibilityPtr>("fog");
}

UnitInfoPtr UnitPtr::Info() const
{
    return Object<UnitInfoPtr>("info");
}

UnitMovementPtr UnitPtr::Movement() const
{
    return Object<UnitMovementPtr>("movement");
}

UnitPlacementPtr UnitPtr::Placement() const
{
    return Object<UnitPlacementPtr>("placement");
}

TagRefArrayPtr UnitPtr::Tags() const
{
    return ObjectRefArray<TagRefArrayPtr>("tags");
}

Phoenix::LDS::ValuePtr UnitPtr::SelectionCircleScale() const
{
    return Value<Phoenix::Value>("selection_circle_scale");
}

UnitSupplyPtr UnitPtr::Supply() const
{
    return Object<UnitSupplyPtr>("supply");
}

UnitVisionPtr UnitPtr::Vision() const
{
    return Object<UnitVisionPtr>("vision");
}

VitalStatsPairArrayPtr UnitPtr::Vitals() const
{
    return ObjectArray<VitalStatsPairArrayPtr>("vitals");
}

WeaponRefArrayPtr UnitPtr::Weapons() const
{
    return ObjectRefArray<WeaponRefArrayPtr>("weapons");
}
