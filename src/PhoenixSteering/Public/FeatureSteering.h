
#pragma once

#include "DLLExport.h"
#include "Features.h"
#include "SteeringSystem.h"
#include "FixedPoint/FixedVector.h"

namespace Phoenix::ECS
{
    struct EntityId;
}

namespace Phoenix::Steering
{
    struct FeatureSteeringDynamicBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_DYNAMIC(FeatureSteeringDynamicBlock)
    };

    class PHOENIX_STEERING_API FeatureSteering : public IFeature
    {
        PHX_FEATURE_BEGIN(FeatureSteering)
            FEATURE_WORLD_BLOCK(FeatureSteeringDynamicBlock)
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

    private:

        void Initialize() override;
        void Shutdown() override;

        bool OnHandleWorldAction(WorldRef world, const FeatureActionArgs& action) override;

        TSharedPtr<SteeringSystem> SteeringSystem;
    };
}
