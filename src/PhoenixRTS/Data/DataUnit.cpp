
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
    success = dataPtr.Supply().TryReadObject(queryContext, outItem.Supply) && success;
    success = dataPtr.Vision().TryReadObject(queryContext, outItem.Vision) && success;
    success = dataPtr.Weapons().GetResolvedObjects(queryContext, outItem.Weapons) && success;

    return success;
}

UnitPtr::UnitPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

Phoenix::LDS::TLDSObjectRefPtr<UnitActorPtr> UnitPtr::Actor() const
{
    return ObjectRef<UnitActor>("actor");
}

UnitArmorPtr UnitPtr::Armor() const
{
    return Object<UnitArmor>("armor");
}

Phoenix::LDS::TLDSObjectRefArrayPtr<BuffPtr> UnitPtr::Buffs() const
{
    return ObjectRefArray<Buff>("buffs");
}

UnitBuildPtr UnitPtr::BuildStats() const
{
    return Object<UnitBuild>("build_stats");
}

UnitCargoPtr UnitPtr::CargoStats() const
{
    return Object<UnitCargo>("cargo");
}

Phoenix::LDS::TLDSEnumFlagsPtr<ECollisionFlags> UnitPtr::CollisionFlags() const
{
    return EnumFlags<ECollisionFlags>("collision");
}

Phoenix::LDS::TLDSObjectArrayPtr<Command> UnitPtr::Commands() const
{
    return ObjectArray<Command>("commands");
}

Phoenix::LDS::TLDSObjectArrayPtr<Component> UnitPtr::Components() const
{
    return ObjectArray<Command>("components");
}

UnitDeathPtr UnitPtr::DeathStats() const
{
    return Object<UnitDeath>("death_stats");
}

UnitEffectsPtr UnitPtr::Effects() const
{
    return Object<UnitEffects>("death_stats");
}

Phoenix::LDS::TLDSObjectRefPtr<FactionPtr> UnitPtr::Faction() const
{
    return ObjectRef<Data::Faction>("faction");
}

UnitFlagsPtr UnitPtr::Flags() const
{
    return Object<UnitFlags>("flags");
}

FogVisibilityPtr UnitPtr::Fog() const
{
    return Object<FogVisibility>("fog");
}

UnitInfoPtr UnitPtr::Info() const
{
    return Object<UnitInfo>("info");
}

UnitMovementPtr UnitPtr::Movement() const
{
    return Object<UnitMovement>("movement");
}

UnitPlacementPtr UnitPtr::Placement() const
{
    return Object<UnitPlacement>("placement");
}

Phoenix::LDS::TLDSObjectRefArrayPtr<TagPtr> UnitPtr::Tags() const
{
    return ObjectRefArray<Tag>("tags");
}

UnitSupplyPtr UnitPtr::Supply() const
{
    return Object<UnitSupply>("supply");
}

UnitVisionPtr UnitPtr::Vision() const
{
    return Object<UnitVision>("vision");
}

UnitVitalsPtr UnitPtr::Vitals() const
{
    return Object<UnitVitals>("vitals");
}

Phoenix::LDS::TLDSObjectRefArrayPtr<WeaponPtr> UnitPtr::Weapons() const
{
    return ObjectRefArray<Weapon>("weapons");
}
