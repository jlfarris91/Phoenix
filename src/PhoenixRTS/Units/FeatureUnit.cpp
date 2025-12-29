
#include "PhoenixRTS/Units/FeatureUnit.h"

#include "PhoenixSim/LDS/FeatureLDS.h"
#include "PhoenixSim/ECS/FeatureECS.h"

#include "PhoenixPhysics/FeaturePhysics.h"
#include "PhoenixPhysics/BodyComponent.h"

#include "PhoenixSteering/SteeringComponent.h"

#include "PhoenixRTS/Abilities/AbilityHandler.h"
#include "PhoenixRTS/Abilities/FeatureAbilities.h"
#include "PhoenixRTS/Data/DataUnit.h"
#include "PhoenixRTS/Units/UnitComponent.h"
#include "PhoenixRTS/Units/UnitSystem.h"
#include "PhoenixRTS/Vitals/VitalComponents.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::LDS;
using namespace Phoenix::Physics;
using namespace Phoenix::Steering;
using namespace Phoenix::RTS;

FeatureUnit::FeatureUnit()
{
    FEATURE_CHANNEL(FeatureChannels::HandleWorldAction)
}

UnitId FeatureUnit::SpawnUnit(
    WorldRef world,
    const FName& unitData,
    uint8 owner,
    const Vec2& pos,
    Angle facing,
    const SpawnUnitArgs& args)
{
    UnitId unitId = UnitId(FeatureECS::AcquireEntity(world, "Unit"_n));
    if (unitId == EntityId::Invalid)
        return {};

    const ILDSQueryContext& lds = *FeatureLDS::StaticGetWorldQueryContext(world);

    Data::UnitPtr dataPtr(unitData);

    UnitComponent* unitComponent = FeatureECS::GetOrAddComponent<UnitComponent>(world, unitId);
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

    // TODO (jfarris): hate that we hard-code the vitals component. What if I don't care about energy or shield?
    dataPtr.Vitals().ForEachItem(lds, [&](const Data::VitalStatsPairPtr& vitalStatsPair)
    {
        Data::VitalPtr vitalPtr = vitalStatsPair.Vital().ResolveObject(lds);
        if (!vitalPtr.Exists(lds))
        {
            return;
        }

        Data::VitalComponentPtr vitalComponentPtr = vitalPtr.Component().ResolveObject(lds);
        if (!vitalComponentPtr.Exists(lds))
        {
            return;
        }

        // TODO (jfarris): remove this hard-coded component initialization
        switch (vitalPtr.GetObjectId())
        {
            case "HealthVital"_n:
                {
                    if (HealthComponent* healthComp = FeatureECS::GetOrAddComponent<HealthComponent>(world, unitId))
                    {
                        Data::VitalStats vitalStats = vitalStatsPair.Stats().ReadObject(lds);
                        healthComp->Health.Current = vitalStats.Starting;
                        healthComp->Health.Max = vitalStats.Max;
                        healthComp->Health.Regen = vitalStats.Regen;
                    }
                    break;
                }
        }
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
    return 0;
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
    return !UnitIsDead(world, unit);
}

bool FeatureUnit::UnitIsDead(WorldConstRef world, UnitId unit)
{
    return FeatureECS::GetBlackboardValue<bool>(world, unit, "dead"_n);
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
    return FeatureECS::SetBlackboardValue(world, unit, "target_scan_level"_n, scanLevel);
}

bool FeatureUnit::ResetTargetScanLevel(WorldRef world, UnitId unit)
{
    // TODO (jfarris): pull this default scan level value from unit data?
    return SetTargetScanLevel(world, unit, ETargetScanLevel::Offensive);
}

int32 FeatureUnit::GetAttackTargetPriority(WorldConstRef world, UnitId unit)
{
    return 0;
}

Time FeatureUnit::GetExpirationTime(WorldRef world, UnitId unit)
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

bool FeatureUnit::HasExpired(WorldRef world, UnitId unit)
{
    Time expirationTime = GetExpirationTime(world, unit);
    return expirationTime > 0 && world.GetSimTime() >= expirationTime;
}

uint32 FeatureUnit::QueryUnitsInRange(
    WorldConstRef world,
    const Vec2& pos,
    Distance range,
    TArray2<UnitId>& outUnits,
    const UnitRangeQueryArgs& args)
{
    EntityRangeQueryArgs rangeQueryArgs;
    rangeQueryArgs.Kinds = { "Unit"_n };

    TArray<EntityTransform> entities;
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

        outUnits.PushBack(entityAsUnit);

        if (++numUnits >= args.MaxNum)
        {
            break;
        }
    }

    return numUnits;
}

void FeatureUnit::Initialize(const TSharedPtr<Phoenix::Session>& session)
{
    IFeature::Initialize(session);

    UnitSystem = MakeShared<RTS::UnitSystem>();

    TSharedPtr<FeatureECS> featureECS = Session->GetFeatureSet()->GetFeature<FeatureECS>();
    featureECS->RegisterSystem(UnitSystem);
}

void FeatureUnit::Shutdown()
{
    if (TSharedPtr<FeatureECS> featureECS = Session->GetFeatureSet()->GetFeature<FeatureECS>())
    {
        featureECS->UnregisterSystem(UnitSystem);
    }

    UnitSystem.reset();
}

bool FeatureUnit::OnHandleWorldAction(WorldRef world, const FeatureActionArgs& args)
{
    if (args.Action.Verb == "spawn_entity"_n)
    {
        FName unitData = args.Action.Data[0].Name;
        Distance x = args.Action.Data[1].Distance;
        Distance y = args.Action.Data[2].Distance;
        Angle facing = args.Action.Data[3].Degrees;
        uint32 num = args.Action.Data[4].UInt32;

        SpawnUnits(world, num, unitData, 0, { x, y }, facing);
        return true;
    }

    return false;
}
