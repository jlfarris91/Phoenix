
#pragma once

#include "PhoenixSteering/DLLExport.h"
#include "PhoenixSim/ECS/Component.h"
#include "PhoenixSim/ECS/EntityId.h"
#include "PhoenixSim/FixedPoint/FixedVector.h"

namespace Phoenix::Steering
{
    enum class PHOENIX_STEERING_API ESteerMode
    {
        Idle,
        Move,
        Turn
    };

    enum class PHOENIX_STEERING_API ESteerFlags
    {
        None = 0,
        SeekingGoal = 1,
        ArrivedAtGoal = 2,
        Active = 4,
        LockFacing = 8,
        FailedPathPlan = 16
    };

    struct PHOENIX_STEERING_API SteeringComponent : ECS::IComponent
    {
        PHX_ECS_DECLARE_COMPONENT_BEGIN(SteeringComponent)
            PHX_REGISTER_FIELD(ESteerMode, Mode)
            PHX_REGISTER_FIELD(ESteerFlags, Flags)
            PHX_REGISTER_FIELD(uint32, CollisionMask)
            PHX_REGISTER_FIELD(ECS::EntityId, GoalEntity)
            PHX_REGISTER_FIELD(Vec2, GoalPos)
            PHX_REGISTER_FIELD(Distance, Slack)
            PHX_REGISTER_FIELD(Vec2, Velocity)
            PHX_REGISTER_FIELD(Vec2, PreviousPos)
            PHX_REGISTER_FIELD(Vec2, BestPos)
            PHX_REGISTER_FIELD(Distance, InnerRadius)
            PHX_REGISTER_FIELD(Distance, OuterRadius)
            PHX_REGISTER_FIELD(Distance, ArrivalRange)
            PHX_REGISTER_FIELD(Distance, ArrivalRange)
            PHX_REGISTER_FIELD(Distance, MaxSpeed)
            PHX_REGISTER_FIELD(Time, TurnRateIdle)
            PHX_REGISTER_FIELD(Time, TurnRateMoving)
            PHX_REGISTER_FIELD(Time, AccelerationTime)
            PHX_REGISTER_FIELD(Time, DecelerationTime)
            PHX_REGISTER_FIELD(Distance, AvoidanceRadius)
            PHX_REGISTER_FIELD(Time, SeparationDelay)
            PHX_REGISTER_FIELD(Distance, SeparationRadius)
            PHX_REGISTER_FIELD(Value, SeparationStrength)
        PHX_ECS_DECLARE_COMPONENT_END()

        ESteerMode Mode = ESteerMode::Idle;
        ESteerFlags Flags = ESteerFlags::Active;

        uint32 CollisionMask = 0;

        ECS::EntityId GoalEntity;
        Vec2 GoalPos;

        Vec2 StepPos[2];
        Distance Slack;

        Vec2 Velocity;
        Vec2 PreviousPos;
        Vec2 BestPos;

        Distance InnerRadius;
        Distance OuterRadius;
        Distance ArrivalRange;

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
