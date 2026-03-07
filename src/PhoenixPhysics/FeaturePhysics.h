#pragma once

#include "PhoenixSim/Containers/FixedArray.h"
#include "PhoenixSim/Containers/FixedSet.h"
#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/ECS/System.h"
#include "PhoenixSim/FixedPoint/FixedPoint.h"
#include "PhoenixSim/FixedPoint/FixedVector.h"
#include "PhoenixSim/FixedPoint/FixedLine.h"
#include "PhoenixPhysics/PhysicsSystem.h"

#ifndef PHX_PHS_MAX_CONTACTS_PER_ENTITY
#define PHX_PHS_MAX_CONTACTS_PER_ENTITY 8
#endif

#ifndef PHX_PHS_MAX_CONTACTS
#define PHX_PHS_MAX_CONTACTS (PHX_ECS_MAX_ENTITIES * PHX_PHS_MAX_CONTACTS_PER_ENTITY)
#endif

namespace Phoenix::Physics
{
    struct BodyComponent;

    struct PHOENIX_PHYSICS_API CollisionLine
    {
        Line2 Line;
    };

    struct PHOENIX_PHYSICS_API EntityBody
    {
        ECS::EntityId EntityId;
        ECS::TransformComponent* TransformComponent;
        BodyComponent* BodyComponent;
        uint64 ZCode;
    };

    struct PHOENIX_PHYSICS_API ContactPair
    {
        uint64 Key = 0;
        ECS::TransformComponent* TransformA;
        BodyComponent* BodyA;
        ECS::TransformComponent* TransformB;
        BodyComponent* BodyB;
    };

    struct PHOENIX_PHYSICS_API Contact
    {
        uint32 ContactPair = Index<uint32>::None;
        uint8 RefCount = 0;
        Vec2 Normal;
        Value EffMass = 0;
        Value Bias = 0;
        Value Impulse = 0;
    };

    struct PHOENIX_PHYSICS_API ContactPairHasher
    {
        uint64 operator()(uint64 v) const
        {
            // Murmur hash
            uint64_t h = v;
            h ^= h >> 33;
            h *= 0xff51afd7ed558ccduLL;
            h ^= h >> 33;
            h *= 0xc4ceb9fe1a85ec53uLL;
            h ^= h >> 33;
            return h;
        }
    };

    struct PHOENIX_PHYSICS_API FeaturePhysicsDynamicBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK(FeaturePhysicsDynamicBlock)
    };

    struct PHOENIX_PHYSICS_API FeaturePhysicsScratchBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK(FeaturePhysicsScratchBlock);

        TInlineArray<EntityBody, PHX_ECS_MAX_ENTITIES> SortedEntities;
        TAtomic<uint32> SortedEntityCount = 0;

        TInlineArray<ContactPair, PHX_PHS_MAX_CONTACTS> ContactPairs;
        TAtomic<uint32> ContactPairsCount = 0;

        TInlineMap<uint64, uint32, PHX_PHS_MAX_CONTACTS> ContactPairSet;

        int32 ContactFreeHead = INDEX_NONE;

        TInlineArray<Contact, PHX_PHS_MAX_CONTACTS> Contacts;
        TInlineArray<CollisionLine, 1000> CollisionLines;
    };

    class PHOENIX_PHYSICS_API FeaturePhysics : public IFeature
    {
        PHX_DECLARE_FEATURE_TYPE(FeaturePhysics)

    public:

        FeaturePhysics();

        void Initialize(const TSharedPtr<Phoenix::Session>& session) override;

        static void QueryEntitiesInRange(WorldConstRef& world, const Vec2& pos, Distance range, TVector<EntityBody>& outEntities);

        static void AddExplosionForceToEntitiesInRange(
            WorldRef& world,
            const Vec2& pos,
            Distance range,
            Value force);

        static void AddForce(WorldRef& world, ECS::EntityId entityId, const Vec2& force);

    protected:

        bool OnHandleWorldAction(WorldRef world, const FeatureActionArgs& action) override;

        TSharedPtr<PhysicsSystem> PhysicsSystem;
    };
}