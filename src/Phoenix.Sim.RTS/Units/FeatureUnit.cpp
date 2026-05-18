#include "Phoenix.Sim.RTS/Units/FeatureUnit.h"
#include "Phoenix/Reflection/Registration.h"

#include "Phoenix.Sim.LDS/FeatureLDS.h"
#include "Phoenix.Sim.ECS/FeatureECS.h"
#include "Phoenix.Sim/Logging.h"
#include "Phoenix.Sim/Session.h"

#include "Phoenix.Sim.Physics/FeaturePhysics.h"
#include "Phoenix.Sim.Physics/BodyComponent.h"

#include "Phoenix.Sim.Steering/SteeringComponent.h"

#include "Phoenix.Sim.RTS/Abilities/AbilityHandler.h"
#include "Phoenix.Sim.RTS/Abilities/FeatureAbilities.h"
#include "Phoenix.Sim.RTS/Data/DataUnit.h"
#include "Phoenix.Sim.RTS/Orders/FeatureOrders.h"
#include "Phoenix.Sim.RTS/Units/UnitComponent.h"
#include "Phoenix.Sim.RTS/Units/UnitSystem.h"
#include "Phoenix.Sim.RTS/Vitals/FeatureVitals.h"
#include "Phoenix.Sim.RTS/Vitals/VitalComponent.h"
#include "Phoenix.Sim.Steering/FeatureSteering.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::LDS;
using namespace Phoenix::Physics;
using namespace Phoenix::Steering;
using namespace Phoenix::RTS;

UnitId FeatureUnit::SpawnUnit(
    WorldRef world,
    const FName& unitData,
    uint8 owner,
    const Vec2& pos,
    Angle facing,
    const SpawnUnitArgs& args)
{
    UnitId unitId = UnitId(FeatureECS::StaticAcquireEntity(world, "Unit"_n));
    if (unitId == EntityId::Invalid)
        return {};

    const ILDSQueryContext& lds = *FeatureLDS::StaticGetWorldQueryContext(world);

    Data::UnitPtr dataPtr(unitData);

    UnitComponent* unitComponent = FeatureECS::GetOrAddComponent<UnitComponent>(world, unitId);
    if (!unitComponent)
    {
        PHX_DEBUG_BREAK();
    }
    unitComponent->OwningPlayer = owner;
    unitComponent->UnitData = unitData;

    // TODO (jfarris): find valid placement at position
    TransformComponent* transformComp = FeatureECS::GetOrAddComponent<TransformComponent>(world, unitId);
    transformComp->Transform.Position = pos;
    transformComp->Transform.Rotation = facing;

    // TODO (jfarris): take from a body component data object?
    BodyComponent* bodyComp = FeatureECS::GetOrAddComponent<BodyComponent>(world, unitId);
    bodyComp->CollisionMask = (uint16)dataPtr.CollisionFlags().GetValue(lds, Data::ECollisionFlags::None);
    bodyComp->Radius = dataPtr.Placement().InnerRadius().GetValue(lds);
    bodyComp->InvMass = OneDivBy<Value>(1.0f);
    bodyComp->LinearDamping = 10.0f;
    
    SteeringComponent* steeringComp = FeatureECS::GetOrAddComponent<SteeringComponent>(world, unitId);
    steeringComp->CollisionMask = (uint32)dataPtr.CollisionFlags().GetValue(lds);
    steeringComp->InnerRadius = dataPtr.Placement().InnerRadius().GetValue(lds);
    steeringComp->OuterRadius = dataPtr.Placement().OuterRadius().GetValue(lds);

    Data::FootprintPtr footprint;
    if (dataPtr.Placement().Footprint().TryResolveObject(lds, footprint))
    {
        FeatureSteering::SetHolding(world, unitId, true);
    }

    // Add vitals and their initial values
    dataPtr.Vitals().ForEachItem(lds, [&](const Data::VitalStatsPairPtr& vitalStatsPair)
    {
        Data::VitalPtr vitalPtr = vitalStatsPair.Vital().ResolveObject(lds);
        if (!vitalPtr.Exists(lds))
        {
            return;
        }

        FName vitalId = vitalStatsPair.Vital().GetReferenceId(lds);
        Data::VitalStats vitalStats = vitalStatsPair.Stats().ReadObject(lds);

        Vital initial;
        initial.Current = vitalStats.Starting;
        initial.Max = vitalStats.Max;
        initial.Regen = vitalStats.Regen;

        FeatureVitals::AddVital(world, unitId, vitalId, initial);
    });

    // Add initial tags
    dataPtr.Tags().ForEachResolvedItemObject(lds, [&](const Data::TagPtr& tag)
    {
       FeatureECS::AddTag(world, unitId, tag.GetObjectId()); 
    });

    // TODO (jfarris): don't need this since the app will be able to read this value
    Data::UnitActorPtr actorPtr = dataPtr.Actor().ResolveObject(lds);
    Color color = Color(actorPtr.Tint().GetValue(lds, Color::White));
    FeatureECS::SetBlackboardValue(world, unitId, "actor_tint"_n, color);

    // TODO (jfarris): dispatch with a unit spawned event instead
    FeatureAbilities::AddAbilitiesFromData(world, unitId, unitData);

    ResetTargetScanLevel(world, unitId);

    return unitId;
}

uint32 FeatureUnit::SpawnUnits(
    WorldRef world,
    uint32 num,
    const FName& unitData,
    uint8 owner,
    const Vec2& pos,
    Angle facing,
    const SpawnUnitArgs& args)
{
    uint32 count = 0;
    for (uint32 i = 0; i < num; ++i)
    {
        UnitId unit = SpawnUnit(world, unitData, owner, pos, facing, args);
        if (FeatureECS::IsEntityValid(world, unit))
        {
            ++count;
        }
    }
    return count;
}

bool FeatureUnit::IsUnitEntity(WorldConstRef world, EntityId entity)
{
    const Entity* ptr = FeatureECS::GetEntityPtr(world, entity);
    return ptr && ptr->Kind == "Unit"_n;
}

FName FeatureUnit::GetUnitDataId(WorldConstRef world, UnitId unit)
{
    const UnitComponent* unitComp = FeatureECS::GetComponent<UnitComponent>(world, unit);
    return unitComp ? unitComp->UnitData : FName::None;
}

RTS::Data::UnitPtr FeatureUnit::GetUnitData(WorldConstRef world, UnitId unit)
{
    return Data::UnitPtr(GetUnitDataId(world, unit));
}

uint8 FeatureUnit::GetOwningPlayer(WorldConstRef world, UnitId unit)
{
    const UnitComponent* unitComp = FeatureECS::GetComponent<UnitComponent>(world, unit);
    return unitComp ? unitComp->OwningPlayer : 0;
}

uint8 FeatureUnit::GetOwningTeam(WorldConstRef world, UnitId unit)
{
    return GetOwningPlayer(world, unit);
}

bool FeatureUnit::UnitCanMove(WorldConstRef world, UnitId unit)
{
    return true;
}

bool FeatureUnit::UnitCanTurn(WorldConstRef world, UnitId unit)
{
    return true;
}

bool FeatureUnit::UnitIsImmobilized(WorldConstRef world, UnitId unit)
{
    return false;
}

bool FeatureUnit::UnitIsAlive(WorldConstRef world, UnitId unit)
{
    return FeatureECS::IsEntityValid(world, unit) && !UnitIsDead(world, unit);
}

bool FeatureUnit::UnitIsDead(WorldConstRef world, UnitId unit)
{
    return FeatureECS::GetBlackboardValue<bool>(world, unit, "status_dead"_n);
}

bool FeatureUnit::UnitIsHidden(WorldConstRef world, UnitId unit)
{
    return false;
}

bool FeatureUnit::UnitIsDetected(WorldConstRef world, UnitId unit, UnitId target)
{
    return true;
}

bool FeatureUnit::UnitIsCargo(WorldConstRef world, UnitId unit)
{
    return false;
}

bool FeatureUnit::UnitIsDormant(WorldConstRef world, UnitId unit)
{
    return false;
}

bool FeatureUnit::UnitCanReceiveCommands(WorldConstRef world, UnitId unit)
{
    return !UnitIsDead(world, unit);
}

ETargetScanLevel FeatureUnit::GetTargetScanLevel(WorldConstRef world, UnitId unit)
{
    return FeatureECS::GetBlackboardValue<ETargetScanLevel>(world, unit, "target_scan_level"_n);
}

bool FeatureUnit::SetTargetScanLevel(WorldRef world, UnitId unit, ETargetScanLevel scanLevel)
{
    LogVerbose("Setting {0} scan level to {1}", (uint32)unit, (uint8)scanLevel);
    return FeatureECS::SetBlackboardValue(world, unit, "target_scan_level"_n, scanLevel);
}

bool FeatureUnit::ResetTargetScanLevel(WorldRef world, UnitId unit)
{
    // TODO (jfarris): pull this default scan level value from unit data?
    FeatureECS::SetBlackboardValue(world, unit, "NextTargetScanTime"_n, Time(0));
    return SetTargetScanLevel(world, unit, ETargetScanLevel::Offensive);
}

int32 FeatureUnit::GetAttackTargetPriority(WorldConstRef world, UnitId unit)
{
    return 0;
}

Time FeatureUnit::GetExpirationTime(WorldConstRef world, UnitId unit)
{
    return FeatureECS::GetBlackboardValue<Time>(world, unit, "expiration_time"_n);
}

bool FeatureUnit::SetExpirationTimer(WorldRef world, UnitId unit, Time expirationTime)
{
    return FeatureECS::SetBlackboardValue<Time>(world, unit, "expiration_time"_n, world.GetSimTime() + expirationTime);
}

bool FeatureUnit::ClearExpirationTimer(WorldRef world, UnitId unit)
{
    return FeatureECS::RemoveBlackboardValue(world, unit, "expiration_time"_n);
}

bool FeatureUnit::HasExpired(WorldConstRef world, UnitId unit)
{
    Time expirationTime = GetExpirationTime(world, unit);
    return expirationTime > 0 && world.GetSimTime() >= expirationTime;
}

uint32 FeatureUnit::QueryUnitsInRange(
    WorldConstRef world,
    const Vec2& pos,
    Distance range,
    std::vector<UnitId>& outUnits,
    const UnitRangeQueryArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    EntityRangeQueryArgs rangeQueryArgs;
    rangeQueryArgs.Kinds = { "Unit"_n };

    std::vector<EntityTransform> entities;
    FeatureECS::QueryEntitiesInRange(world, pos, range, entities);

    uint32 numUnits = 0;
    for (const EntityTransform& entity : entities)
    {
        UnitId entityAsUnit = UnitId(entity.EntityId);

        if (std::ranges::find(args.Exclude, entityAsUnit) != args.Exclude.end())
        {
            continue;
        }

        if (HasNoneFlags(args.Flags, EUnitQueryFlags::Alive) && UnitIsAlive(world, entityAsUnit))
        {
            continue;
        }

        if (HasNoneFlags(args.Flags, EUnitQueryFlags::Dead) && UnitIsDead(world, entityAsUnit))
        {
            continue;
        }

        if (HasNoneFlags(args.Flags, EUnitQueryFlags::Cargo) && UnitIsCargo(world, entityAsUnit))
        {
            continue;
        }

        if (HasNoneFlags(args.Flags, EUnitQueryFlags::Hidden) && UnitIsHidden(world, entityAsUnit))
        {
            continue;
        }

        TeamMask teamMask = 1ull << GetOwningTeam(world, entityAsUnit);
        if ((args.TeamMask & teamMask) == 0)
        {
            continue;
        }

        outUnits.push_back(entityAsUnit);

        if (++numUnits >= args.MaxNum)
        {
            break;
        }
    }

    return numUnits;
}

void FeatureUnit::OnUnitKilled(WorldRef world, UnitId unit, EntityId source)
{
    if (UnitIsDead(world, unit))
    {
        return;
    }

    FeatureECS::SetBlackboardValue(world, unit, "status_dead"_n, true);

    const ILDSQueryContext& lds = *FeatureLDS::StaticGetWorldQueryContext(world);

    Data::UnitPtr unitData = GetUnitData(world, unit);
    Time expirationTime = unitData.DeathStats().ExpirationTime().GetValue(lds);
    SetExpirationTimer(world, unit, expirationTime);

    FeatureOrders::ClearOrderQueue(world, unit);

    LogInfo("Unit {0} was killed by {1}", (uint32)unit, (uint32)source);
}

void FeatureUnit::Initialize(const std::shared_ptr<Phoenix::Session>& session)
{
    IFeature::Initialize(session);

    UnitSystem = std::make_shared<RTS::UnitSystem>();

    std::shared_ptr<FeatureECS> featureECS = Session->GetFeatureSet()->GetFeature<FeatureECS>();
    featureECS->RegisterSystem(UnitSystem);
}

void FeatureUnit::Shutdown()
{
    if (auto featureECS = Session->GetFeatureSet()->GetFeature<FeatureECS>())
    {
        featureECS->UnregisterSystem(UnitSystem);
    }

    UnitSystem.reset();

    IFeature::Shutdown();
}

bool FeatureUnit::OnHandleWorldAction(WorldRef world, const FeatureActionArgs& args)
{
    if (args.Action.Verb == "spawn_entity"_n)
    {
        FName unitData = args.Action.Args[0].AsName;
        Distance x = args.Action.Args[1].AsDistance;
        Distance y = args.Action.Args[2].AsDistance;
        Angle facing = args.Action.Args[3].AsDegrees;
        uint32 num = args.Action.Args[4].AsUInt32;
        uint8 owner = args.Action.Args[5].AsUInt32;

        SpawnUnits(world, num, unitData, owner, { x, y }, facing);
        return true;
    }

    return false;
}
