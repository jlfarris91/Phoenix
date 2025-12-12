
#pragma once

#include "DLLExport.h"
#include "FeatureECS.h"
#include "Features.h"
#include "SteeringSystem.h"
#include "Containers/FixedArray.h"
#include "FixedPoint/FixedVector.h"

namespace Phoenix::ECS
{
    struct EntityId;
}

namespace Phoenix::Steering
{
    struct SteeringComponent;

    struct FeatureSteeringDynamicBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_DYNAMIC(FeatureSteeringDynamicBlock)
    };

    struct SortedEntity
    {
        ECS::EntityId EntityId;
        ECS::TransformComponent* TransformComponent;
        SteeringComponent* SteeringComponent;
        uint64 ZCode;
    };

    struct FeatureSteeringScratchBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_SCRATCH(FeatureSteeringScratchBlock)

        TFixedArray<SortedEntity, PHX_ECS_MAX_ENTITIES> SortedEntities;
        TAtomic<uint32> SortedEntityCount = 0;
    };

    struct SteeringSpeedArgs
    {
        TOptional<Speed> MaxSpeed;
        TOptional<Time> AccelerationTime;
        TOptional<Time> DecelerationTime;
    };

    class PHOENIX_STEERING_API FeatureSteering : public IFeature
    {
        PHX_FEATURE_BEGIN(FeatureSteering)
            FEATURE_WORLD_BLOCK(FeatureSteeringDynamicBlock)
            FEATURE_WORLD_BLOCK(FeatureSteeringScratchBlock)
            FEATURE_CHANNEL(FeatureChannels::HandleWorldAction)
        PHX_FEATURE_END()

    public:

        // Starts moving an entity towards a location.
        static bool MoveToLocation(WorldRef world, const ECS::EntityId& entity, const Vec2& target);

        // Starts moving an entity towards another entity, following it as long as it is valid.
        static bool FollowEntity(WorldRef world, const ECS::EntityId& entity, const ECS::EntityId& target);

        // Returns true if the entity is currently seeking its goal.
        static bool IsSeekingGoal(WorldRef world, const ECS::EntityId& entity);

        // Returns true if the entity has arrived at its goal.
        static bool HasArrivedAtGoal(WorldRef world, const ECS::EntityId& entity);

        // Stops an entity from seeking its current goal, if there was one.
        static bool Stop(WorldRef world, const ECS::EntityId& entity);

        // Updates the speed properties of a steering component.
        static bool UpdateSpeed(WorldRef world, const ECS::EntityId& entity, const SteeringSpeedArgs& args);

    private:

        void Initialize() override;
        void Shutdown() override;

        bool OnHandleWorldAction(WorldRef world, const FeatureActionArgs& action) override;

        TSharedPtr<SteeringSystem> SteeringSystem;
    };
}
