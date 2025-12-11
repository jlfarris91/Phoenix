
#pragma once

#include "DLLExport.h"
#include "Component.h"
#include "EntityId.h"
#include "FixedPoint/FixedVector.h"

namespace Phoenix::Steering
{
    struct PHOENIX_STEERING_API SteeringComponent : ECS::IComponent
    {
        PHX_ECS_DECLARE_COMPONENT(SteeringComponent)

        // The accumulated steering vector for a given frame.
        Vec2 SteeringVector;

        // The max speed an entity can move not considering pushing forces.
        Speed MaxSpeed;

        // The radius at which to avoid other entities.
        Distance AvoidanceRadius;

        // The time before separation kicks in.
        Time SeparationDelay;

        // The radius used when separating from neighbors.
        Distance SeparationRadius;

        // The strength of the separation force.
        Value SeparationStrength;
    };

    enum class ESeekFlags
    {
        None = 0,
        SeekingGoal = 1,
        ArrivedAtGoal = 2,
    };

    struct PHOENIX_STEERING_API SeekComponent : ECS::IComponent
    {
        PHX_ECS_DECLARE_COMPONENT(SeekComponent)

        ESeekFlags Flags = ESeekFlags::None;
        ECS::EntityId TargetEntity;
        Vec2 TargetPos;
    };

    struct PHOENIX_STEERING_API WanderComponent : ECS::IComponent
    {
        PHX_ECS_DECLARE_COMPONENT(WanderComponent)

        Angle WanderAngle;
        Distance WanderRadius;
        Speed MaxSpeed;
    };
}
