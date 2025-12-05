
#include "Units/FeatureUnit.h"

#include "BodyComponent.h"
#include "FeatureLDS.h"
#include "FeatureECS.h"
#include "FeaturePhysics.h"
#include "../../../PhoenixSteering/Public/SteeringComponent.h"

#include "Data/DataUnit.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::LDS;
using namespace Phoenix::Physics;
using namespace Phoenix::Steering;
using namespace Phoenix::RTS;

Unit FeatureUnit::SpawnUnit(
    WorldRef world,
    const FName& unitData,
    uint32 owner,
    const Vec2& pos,
    Angle facing,
    const SpawnUnitArgs& args)
{
    EntityId entityId = FeatureECS::AcquireEntity(world, "Unit"_n);
    if (entityId == EntityId::Invalid)
        return {};

    LDSQueryContext context = LDSQueryContext::Create(world);

    Data::UnitPtr dataPtr(unitData);

    TransformComponent* transformComp = FeatureECS::GetComponent<TransformComponent>(world, entityId);
    transformComp->Transform.Position = pos;
    transformComp->Transform.Rotation = facing;

    BodyComponent* bodyComp = FeatureECS::GetComponent<BodyComponent>(world, entityId);
    bodyComp->CollisionMask = (uint16)dataPtr.CollisionFlags.GetValue(context, Data::ECollisionFlags::None);
    bodyComp->Radius = dataPtr.Placement.InnerRadius.GetValue(context);

    // TODO (jfarris): take from a body component data object? 
    bodyComp->InvMass = OneDivBy<Value>(1.0f);
    bodyComp->LinearDamping = 10.0f;

    Color color = Color(dataPtr.Actor.ResolveObject(context).Tint.GetValue(context, Color::White));
    FeatureECS::SetBlackboardValue(world, entityId, "Color"_n, color);

    SteeringComponent* steeringComp = FeatureECS::GetComponent<SteeringComponent>(world, entityId);
    steeringComp->AvoidanceRadius = bodyComp->Radius * 2;
    steeringComp->MaxSpeed = 50.0f;

    if (entityId == 1)
    {
        // Steering::WanderComponent* wanderComp = FeatureECS::AddComponent<Steering::WanderComponent>(world, entityId);
        // wanderComp->WanderAngle = ((rand() % RAND_MAX) / (double)RAND_MAX) * DEG_360;
        // wanderComp->WanderRadius = 10.0;
        // wanderComp->MaxSpeed = 5.0;
    }
    else
    {
        SeekComponent* seekComp = FeatureECS::GetComponent<SeekComponent>(world, entityId);
        SetFlagRef(seekComp->Flags, ESeekFlags::Arrive, true); 
        seekComp->TargetEntity = 0;
        seekComp->SlowingDistance = 3;
        seekComp->MaxSpeed = 50.0; 
    }

    return {};
}

uint32 FeatureUnit::SpawnUnits(
    WorldRef world,
    uint32 num,
    const FName& unitData,
    uint32 owner,
    const Vec2& pos,
    Angle facing,
    const SpawnUnitArgs& args)
{
    uint32 count = 0;
    for (uint32 i = 0; i < num; ++i)
    {
        Unit unit = SpawnUnit(world, unitData, owner, pos, facing, args);
        if (FeatureECS::IsEntityValid(world, unit))
        {
            ++count;
        }
    }
    return count;
}

Value FeatureUnit::GetHealth(WorldConstRef world, Unit unit)
{
    return 0.0;
}

bool FeatureUnit::OnHandleWorldAction(WorldRef world, const FeatureActionArgs& action)
{
    if (action.Action.Verb == "spawn_entity"_n)
    {
        FName unitData = action.Action.Data[0].Name;
        Distance x = action.Action.Data[1].Distance;
        Distance y = action.Action.Data[2].Distance;
        Angle facing = action.Action.Data[3].Degrees;
        uint32 num = action.Action.Data[4].UInt32;

        SpawnUnits(world, num, unitData, 0, { x, y }, facing);
        return true;
    }
    return false;
}
