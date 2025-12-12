
#pragma once

#include "DLLExport.h"
#include "Component.h"
#include "EntityId.h"
#include "FixedPoint/FixedVector.h"

namespace Phoenix::Steering
{
    enum class ESteerFlags
    {
        None = 0,
        SeekingGoal = 1,
        ArrivedAtGoal = 2,
        Active = 4
    };

    struct PHOENIX_STEERING_API SteeringComponent : ECS::IComponent
    {
        PHX_ECS_DECLARE_COMPONENT(SteeringComponent)

        ESteerFlags Flags = ESteerFlags::None;

        ECS::EntityId GoalEntity;
        Vec2 GoalPos;

        Vec2 StepPos[2];

        Vec2 Velocity;
        Vec2 PreviousPos;
        uint32 CollisionMask = 0;

        Distance InnerRadius;
        Distance OuterRadius;

        // The max speed an entity can move not considering pushing forces.
        Distance MaxSpeed;

        // The amount of time it takes to turn 180 degrees when idle.
        Time TurnRateIdle;

        // The amount of time it takes to turn 180 degrees when moving.
        Time TurnRateMoving;

        // The amount of time it takes to go from stopped to max speed.
        Time AccelerationTime;

        // The amount of time it takes to go from max speed to stopped.
        Time DecelerationTime;

        // The radius at which to avoid other entities.
        Distance AvoidanceRadius;

        // The time before separation kicks in.
        Time SeparationDelay;

        // The radius used when separating from neighbors.
        Distance SeparationRadius;

        // The strength of the separation force.
        Value SeparationStrength;
    };
}
