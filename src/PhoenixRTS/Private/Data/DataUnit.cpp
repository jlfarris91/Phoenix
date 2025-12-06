
#include "Data/DataUnit.h"

using namespace Phoenix::RTS::Data;

bool Unit::Read(const LDS::LDSReadObjectArgs& context, Unit& outItem)
{
    bool success = true;

    UnitPtr dataPtr(context.Path, context.Flags);
    success = dataPtr.Actor.TryResolveObject(context, outItem.Actor) && success;
    success = dataPtr.Armor.TryReadObject(context, outItem.Armor) && success;
    success = dataPtr.Buffs.GetObjectRefs(context, outItem.Buffs) && success;
    success = dataPtr.BuildStats.TryReadObject(context, outItem.BuildStats) && success;
    success = dataPtr.CargoStats.TryReadObject(context, outItem.CargoStats) && success;
    success = dataPtr.CollisionFlags.TryGetValue(context, outItem.CollisionFlags) && success;
    success = dataPtr.Commands.ReadObjects(context, outItem.Commands) && success;
    success = dataPtr.Components.ReadObjects(context, outItem.Components) && success;
    success = dataPtr.DeathStats.TryReadObject(context, outItem.DeathStats) && success;
    success = dataPtr.Effects.TryReadObject(context, outItem.Effects) && success;
    success = dataPtr.Faction.TryResolveObject(context, outItem.Faction) && success;
    success = dataPtr.Flags.TryReadObject(context, outItem.Flags) && success;
    success = dataPtr.Fog.TryReadObject(context, outItem.Fog) && success;
    success = dataPtr.Info.TryReadObject(context, outItem.Info) && success;
    success = dataPtr.Placement.TryReadObject(context, outItem.Placement) && success;
    success = dataPtr.Tags.GetObjectRefs(context, outItem.Tags) && success;
    success = dataPtr.Supply.TryReadObject(context, outItem.Supply) && success;
    success = dataPtr.Vision.TryReadObject(context, outItem.Vision) && success;
    success = dataPtr.Weapons.GetObjectRefs(context, outItem.Weapons) && success;

    return success;
}

UnitPtr::UnitPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
    , Actor(ObjectRef<UnitActor>("actor"))
    , Armor(Object<UnitArmor>("armor"))
    , Buffs(ObjectRefArray<Buff>("buffs"))
    , BuildStats(Object<UnitBuild>("build_stats"))
    , CargoStats(Object<UnitCargo>("cargo"))
    , CollisionFlags(EnumFlags<ECollisionFlags>("collision_flags"))
    , Commands(ObjectArray<Command>("commands"))
    , Components(ObjectArray<Command>("components"))
    , DeathStats(Object<UnitDeath>("death_stats"))
    , Effects(Object<UnitEffects>("death_stats"))
    , Faction(ObjectRef<Data::Faction>("faction"))
    , Flags(Object<UnitFlags>("flags"))
    , Fog(Object<FogVisibility>("fog"))
    , Info(Object<UnitInfo>("info"))
    , Placement(Object<UnitPlacement>("placement"))
    , Tags(ObjectRefArray<Tag>("tags"))
    , Supply(Object<UnitSupply>("supply"))
    , Vision(Object<UnitVision>("vision"))
    , Vitals(Object<UnitVitals>("vitals"))
    , Weapons(ObjectRefArray<Weapon>("weapons"))
{
}
