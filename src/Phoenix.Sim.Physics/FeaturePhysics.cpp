#include "Phoenix.Sim.Physics/FeaturePhysics.h"

#include "Phoenix/MortonCode.h"
#include "Phoenix/Profiling.h"
#include "Phoenix.Sim/Session.h"
#include "Phoenix.Sim.Physics/BodyComponent.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::Physics;

void FeaturePhysics::Initialize(const std::shared_ptr<Phoenix::Session>& session)
{
    IFeature::Initialize(session);

    PhysicsSystem = std::make_shared<Physics::PhysicsSystem>();

    auto featureECS = Session->GetFeatureSet()->GetFeature<FeatureECS>();
    featureECS->RegisterSystem(PhysicsSystem);
}

void FeaturePhysics::QueryEntitiesInRange(
    WorldConstRef world,
    const Vec2& pos,
    Distance range,
    std::vector<EntityBody>& outEntities)
{
    PHX_PROFILE_ZONE_SCOPED;

    const FeaturePhysicsScratchBlock& scratchBlock = world.GetBlockRef<FeaturePhysicsScratchBlock>();

    TMortonCodeRangeArray ranges;
    
    // Query for overlapping morton ranges
    {
        MortonCodeAABB aabb = ToMortonCodeAABB(pos, range);
        MortonCodeQuery(aabb, ranges);
    }

    std::vector<EntityBody*> overlappingEntities;
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

    std::vector<EntityBody> outEntities;
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
        Vec2 pos = { action.Action.Args[0].AsDistance, action.Action.Args[1].AsDistance };
        Distance range = action.Action.Args[2].AsDistance;
        Value force = action.Action.Args[3].AsDistance;
        AddExplosionForceToEntitiesInRange(world, pos, range, force);

        return true;
    }

    return false;
}
