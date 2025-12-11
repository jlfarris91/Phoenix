
#include "Data/DataUnit.h"

using namespace Phoenix::RTS::Data;

bool Unit::Read(const LDS::LDSReadObjectArgs& args, Unit& outItem)
{
    const LDS::ILDSQueryContext& queryContext = *args.GetQueryContext();

    bool success = true;

    UnitPtr dataPtr = args.CreatePtr<UnitPtr>();
    success = dataPtr.Actor.TryResolveObject(queryContext, outItem.Actor) && success;
    success = dataPtr.Armor.TryReadObject(queryContext, outItem.Armor) && success;
    success = dataPtr.Buffs.GetResolvedObjects(queryContext, outItem.Buffs) && success;
    success = dataPtr.BuildStats.TryReadObject(queryContext, outItem.BuildStats) && success;
    success = dataPtr.CargoStats.TryReadObject(queryContext, outItem.CargoStats) && success;
    success = dataPtr.CollisionFlags.TryGetValue(queryContext, outItem.CollisionFlags) && success;
    success = dataPtr.Commands.ReadObjects(queryContext, outItem.Commands) && success;
    success = dataPtr.Components.ReadObjects(queryContext, outItem.Components) && success;
    success = dataPtr.DeathStats.TryReadObject(queryContext, outItem.DeathStats) && success;
    success = dataPtr.Effects.TryReadObject(queryContext, outItem.Effects) && success;
    success = dataPtr.Faction.TryResolveObject(queryContext, outItem.Faction) && success;
    success = dataPtr.Flags.TryReadObject(queryContext, outItem.Flags) && success;
    success = dataPtr.Fog.TryReadObject(queryContext, outItem.Fog) && success;
    success = dataPtr.Info.TryReadObject(queryContext, outItem.Info) && success;
    success = dataPtr.Movement.TryReadObject(queryContext, outItem.Movement) && success;
    success = dataPtr.Placement.TryReadObject(queryContext, outItem.Placement) && success;
    success = dataPtr.Tags.GetResolvedObjects(queryContext, outItem.Tags) && success;
    success = dataPtr.Supply.TryReadObject(queryContext, outItem.Supply) && success;
    success = dataPtr.Vision.TryReadObject(queryContext, outItem.Vision) && success;
    success = dataPtr.Weapons.GetResolvedObjects(queryContext, outItem.Weapons) && success;

    return success;
}

UnitPtr::UnitPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
    , Actor(ObjectRef<UnitActor>("actor"))
    , Armor(Object<UnitArmor>("armor"))
    , Buffs(ObjectRefArray<Buff>("buffs"))
    , BuildStats(Object<UnitBuild>("build_stats"))
    , CargoStats(Object<UnitCargo>("cargo"))
    , CollisionFlags(EnumFlags<ECollisionFlags>("collision"))
    , Commands(ObjectArray<Command>("commands"))
    , Components(ObjectArray<Command>("components"))
    , DeathStats(Object<UnitDeath>("death_stats"))
    , Effects(Object<UnitEffects>("death_stats"))
    , Faction(ObjectRef<Data::Faction>("faction"))
    , Flags(Object<UnitFlags>("flags"))
    , Fog(Object<FogVisibility>("fog"))
    , Info(Object<UnitInfo>("info"))
    , Movement(Object<UnitMovement>("movement"))
    , Placement(Object<UnitPlacement>("placement"))
    , Tags(ObjectRefArray<Tag>("tags"))
    , Supply(Object<UnitSupply>("supply"))
    , Vision(Object<UnitVision>("vision"))
    , Vitals(Object<UnitVitals>("vitals"))
    , Weapons(ObjectRefArray<Weapon>("weapons"))
{
}
