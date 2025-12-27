
#include "PhoenixPhysics/FeaturePhysics.h"

#include "PhoenixSim/MortonCode.h"
#include "PhoenixSim/Profiling.h"
#include "PhoenixSim/Session.h"
#include "PhoenixPhysics/BodyComponent.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::Physics;

FeaturePhysics::FeaturePhysics()
{
    FEATURE_WORLD_BLOCK(FeaturePhysicsDynamicBlock)
    FEATURE_WORLD_BLOCK(FeaturePhysicsScratchBlock)
    FEATURE_CHANNEL(FeatureChannels::HandleWorldAction)
}

void FeaturePhysics::Initialize(const TSharedPtr<Phoenix::Session>& session)
{
    IFeature::Initialize(session);

    PhysicsSystem = MakeShared<Physics::PhysicsSystem>();

    TSharedPtr<FeatureECS> featureECS = Session->GetFeatureSet()->GetFeature<FeatureECS>();
    featureECS->RegisterSystem(PhysicsSystem);
}

void FeaturePhysics::QueryEntitiesInRange(
    WorldConstRef world,
    const Vec2& pos,
    Distance range,
    TArray<EntityBody>& outEntities)
{
    PHX_PROFILE_ZONE_SCOPED;

    const FeaturePhysicsScratchBlock& scratchBlock = world.GetBlockRef<FeaturePhysicsScratchBlock>();

    TMortonCodeRangeArray ranges;
    
    // Query for overlapping morton ranges
    {
        MortonCodeAABB aabb = ToMortonCodeAABB(pos, range);
        MortonCodeQuery(aabb, ranges);
    }

    TArray<EntityBody*> overlappingEntities;
    ForEachInMortonCodeRanges<EntityBody, &EntityBody::ZCode>(
        scratchBlock.SortedEntities,
        ranges,
        [&](const EntityBody& entityBody)
        {
            outEntities.push_back(entityBody);
        });
}

void FeaturePhysics::AddExplosionForceToEntitiesInRange(WorldRef world, const Vec2& pos, Distance range, Value force)
{
    // Distance rangeSq = range * range;

    TArray<EntityBody> outEntities;
    QueryEntitiesInRange(world, pos, range, outEntities);

    for (const EntityBody& entityBody : outEntities)
    {
        const Vec2& entityPos = entityBody.TransformComponent->Transform.Position;
        Vec2 dir = entityPos - pos;
        Distance dist = dir.Length();
        if (dist < range)
        {
            Value t = 1.0f - dist / range;
            entityBody.BodyComponent->Force += dir.Normalized() * force * t;
        }
    }
}

void FeaturePhysics::AddForce(WorldRef world, EntityId entityId, const Vec2& force)
{
    BodyComponent* bodyComponent = FeatureECS::GetComponent<BodyComponent>(world, entityId);
    if (!bodyComponent)
    {
        return;
    }

    bodyComponent->Force += force;
}

bool FeaturePhysics::OnHandleWorldAction(WorldRef world, const FeatureActionArgs& action)
{
    if (action.Action.Verb == "push_entities_in_range"_n)
    {
        Vec2 pos = { action.Action.Data[0].Distance, action.Action.Data[1].Distance };
        Distance range = action.Action.Data[2].Distance;
        Value force = action.Action.Data[3].Distance;
        AddExplosionForceToEntitiesInRange(world, pos, range, force);

        return true;
    }

    return false;
}
