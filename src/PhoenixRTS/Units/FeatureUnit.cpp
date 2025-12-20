
#include "PhoenixRTS/Units/FeatureUnit.h"

#include "PhoenixSim/LDS/FeatureLDS.h"
#include "PhoenixSim/ECS/FeatureECS.h"

#include "PhoenixPhysics/FeaturePhysics.h"
#include "PhoenixPhysics/BodyComponent.h"

#include "PhoenixSteering/SteeringComponent.h"

#include "PhoenixRTS/Abilities/Ability.h"
#include "PhoenixRTS/Abilities/FeatureAbilities.h"
#include "PhoenixRTS/Commands/Commands.h"
#include "PhoenixRTS/Data/DataUnit.h"
#include "PhoenixRTS/Selection/FeatureSelection.h"
#include "PhoenixRTS/Vitals/VitalsComponent.h"

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
    UnitId unitId = UnitId(FeatureECS::AcquireEntity(world, "Unit"_n));
    if (unitId == EntityId::Invalid)
        return {};

    const ILDSQueryContext& queryContext = *FeatureLDS::StaticGetWorldQueryContext(world);

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
    bodyComp->CollisionMask = (uint16)dataPtr.CollisionFlags().GetValue(queryContext, Data::ECollisionFlags::None);
    bodyComp->Radius = dataPtr.Placement().InnerRadius().GetValue(queryContext);
    bodyComp->InvMass = OneDivBy<Value>(1.0f);
    bodyComp->LinearDamping = 10.0f;

    // TODO (jfarris): hate that we hard-code the vitals component. What if I don't care about energy or shield?
    VitalsComponent* vitalsComp = FeatureECS::GetOrAddComponent<VitalsComponent>(world, unitId);
    Data::UnitVitalsPtr vitalsPtr = dataPtr.Vitals();
    vitalsComp->Health.Current = vitalsPtr.Health.Starting.GetValue(queryContext);
    vitalsComp->Health.Max = vitalsPtr.Health.Max.GetValue(queryContext);
    vitalsComp->Health.Regen = vitalsPtr.Health.Regen.GetValue(queryContext);
    vitalsComp->Energy.Current = vitalsPtr.Energy.Starting.GetValue(queryContext);
    vitalsComp->Energy.Max = vitalsPtr.Energy.Max.GetValue(queryContext);
    vitalsComp->Energy.Regen = vitalsPtr.Energy.Regen.GetValue(queryContext);
    vitalsComp->Shield.Current = vitalsPtr.Shield.Starting.GetValue(queryContext);
    vitalsComp->Shield.Max = vitalsPtr.Shield.Max.GetValue(queryContext);
    vitalsComp->Shield.Regen = vitalsPtr.Shield.Regen.GetValue(queryContext);

    // TODO (jfarris): don't need this since the app will be able to read this value
    Data::UnitActorPtr actorPtr = dataPtr.Actor().ResolveObject(queryContext);
    Color color = Color(actorPtr.Tint.GetValue(queryContext, Color::White));
    FeatureECS::SetBlackboardValue(world, unitId, "actor_tint"_n, color);

    // TODO (jfarris): dispatch with a unit spawned event instead
    FeatureAbilities::AddAbilitiesFromData(world, unitId, unitData);

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

uint8 FeatureUnit::GetOwningPlayer(WorldConstRef world, UnitId unit)
{
    const UnitComponent* unitComp = FeatureECS::GetComponent<UnitComponent>(world, unit);
    return unitComp ? unitComp->OwningPlayer : 0;
}

Value FeatureUnit::GetHealth(WorldConstRef world, UnitId unit)
{
    const VitalsComponent* vitalsComp = FeatureECS::GetComponent<VitalsComponent>(world, unit);
    return vitalsComp ? vitalsComp->Health.Current : 0.0;
}

Value FeatureUnit::GetHealthMax(WorldConstRef world, UnitId unit)
{
    const VitalsComponent* vitalsComp = FeatureECS::GetComponent<VitalsComponent>(world, unit);
    return vitalsComp ? vitalsComp->Health.Max : 0.0;
}

Value FeatureUnit::GetHealthRegen(WorldConstRef world, UnitId unit)
{
    const VitalsComponent* vitalsComp = FeatureECS::GetComponent<VitalsComponent>(world, unit);
    return vitalsComp ? vitalsComp->Health.Regen : 0.0;
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
    return true;
}

bool FeatureUnit::UnitIsDead(WorldConstRef world, UnitId unit)
{
    return false;
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
