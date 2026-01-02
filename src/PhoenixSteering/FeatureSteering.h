
#pragma once

#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/Containers/FixedArray.h"
#include "PhoenixSim/FixedPoint/FixedVector.h"
#include "PhoenixSteering/DLLExport.h"
#include "PhoenixSteering/SteeringComponent.h"
#include "PhoenixSteering/SteeringSystem.h"

namespace Phoenix::ECS
{
    struct EntityId;
}

namespace Phoenix::Steering
{
    struct SteeringComponent;

    struct PHOENIX_STEERING_API FeatureSteeringDynamicBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_DYNAMIC(FeatureSteeringDynamicBlock)
    };

    struct PHOENIX_STEERING_API SortedEntity
    {
        ECS::EntityId EntityId;
        ECS::TransformComponent* TransformComponent;
        SteeringComponent* SteeringComponent;
        uint64 ZCode;
    };

    struct PHOENIX_STEERING_API FeatureSteeringScratchBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_SCRATCH(FeatureSteeringScratchBlock)

        TFixedArray<SortedEntity, PHX_ECS_MAX_ENTITIES> SortedEntities;
        TAtomic<uint32> SortedEntityCount = 0;

        Distance MaxEntityRadius;
    };

    struct PHOENIX_STEERING_API SteeringSpeedArgs
    {
        TOptional<Speed> MaxSpeed;
        TOptional<Time> AccelerationTime;
        TOptional<Time> DecelerationTime;
    };

    struct SteeringRangeQueryArgs
    {
        uint32 CollisionMask = (uint32)-1;
        TArray2<ECS::EntityId> Exclude;
        uint32 MaxNum = 64;
    };

    class PHOENIX_STEERING_API FeatureSteering : public IFeature
    {
        PHX_DECLARE_FEATURE_TYPE(FeatureSteering)

    public:

        FeatureSteering();

        // Starts moving an entity towards a location.
        static bool MoveToLocation(WorldRef world, const ECS::EntityId& entity, const Vec2& target, Distance range);

        // Starts moving an entity towards another entity, following it as long as it is valid.
        static bool FollowEntity(WorldRef world, const ECS::EntityId& entity, const ECS::EntityId& target, Distance range);

        // Returns true if the entity is in the process of moving.
        static bool IsMoving(WorldConstRef world, const ECS::EntityId& entity);

        // Returns true if the entity was in the process of moving and has arrived.
        static bool HasFinishedMoving(WorldConstRef world, const ECS::EntityId& entity);

        // Starts turning an entity to face a target entity, following it as long as it is valid.
        static bool TurnToFace(WorldRef world, const ECS::EntityId& entity, const ECS::EntityId& target);

        // Starts turning an entity to face a target location.
        static bool TurnToFace(WorldRef world, const ECS::EntityId& entity, const Vec2& target);

        // Returns true if an entity is in the process of turning.
        static bool IsTurning(WorldConstRef world, const ECS::EntityId& entity);

        // Returns true if an entity was in the process of turning and has finished.
        static bool HasFinishedTurning(WorldConstRef world, const ECS::EntityId& entity);

        // Returns the current steering mode of an entity if a steering component is present.
        static TOptional<ESteerMode> GetSteeringMode(WorldRef world, const ECS::EntityId& entity);

        // Returns true if the entity is currently seeking its goal.
        static bool IsSeekingGoal(WorldRef world, const ECS::EntityId& entity);

        // Returns true if the entity has arrived at its goal.
        static bool HasArrivedAtGoal(WorldRef world, const ECS::EntityId& entity);

        // Stops an entity from seeking its current goal, if there was one.
        static bool Stop(WorldRef world, const ECS::EntityId& entity);

        // Updates the speed properties of a steering component.
        static bool UpdateSpeed(WorldRef world, const ECS::EntityId& entity, const SteeringSpeedArgs& args);

        static Distance GetEntityInnerRadius(WorldConstRef world, const ECS::EntityId& entity);

        static Distance GetEntityOuterRadius(WorldConstRef world, const ECS::EntityId& entity);

        static uint32 QueryEntitiesInRange(
            WorldConstRef world,
            const Vec2& pos,
            Distance range,
            TArray2<const SortedEntity*>& outEntities,
            const SteeringRangeQueryArgs& args = {});

    private:

        void Initialize(const TSharedPtr<Phoenix::Session>& session) override;
        void Shutdown() override;

        bool OnHandleWorldAction(WorldRef world, const FeatureActionArgs& action) override;

        TSharedPtr<SteeringSystem> SteeringSystem;
    };
}
