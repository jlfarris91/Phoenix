
#pragma once

#include "Phoenix.Sim.Steering/DLLExport.h"
#include "Phoenix.Sim/ECS/Component.h"
#include "Phoenix.Sim/ECS/EntityId.h"
#include "Phoenix.Sim/FixedPoint/FixedVector.h"

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
        FailedPathPlan = 16,
        Holding = 32
    };

    struct PHOENIX_STEERING_API SteeringComponent : ECS::IComponent
    {
        PHX_DECLARE_TYPE(SteeringComponent, Phoenix::ECS::IComponent)

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

PHX_DEFINE_TYPE(Phoenix::Steering::SteeringComponent)
{
    registration
        .Field("Mode",              &Steering::SteeringComponent::Mode)
        .Field("Flags",             &Steering::SteeringComponent::Flags)
        .Field("CollisionMask",     &Steering::SteeringComponent::CollisionMask)
        .Field("GoalEntity",        &Steering::SteeringComponent::GoalEntity)
        .Field("GoalPos",           &Steering::SteeringComponent::GoalPos)
        .Field("Slack",             &Steering::SteeringComponent::Slack)
        .Field("Velocity",          &Steering::SteeringComponent::Velocity)
        .Field("PreviousPos",       &Steering::SteeringComponent::PreviousPos)
        .Field("BestPos",           &Steering::SteeringComponent::BestPos)
        .Field("InnerRadius",       &Steering::SteeringComponent::InnerRadius)
        .Field("OuterRadius",       &Steering::SteeringComponent::OuterRadius)
        .Field("ArrivalRange",      &Steering::SteeringComponent::ArrivalRange)
        .Field("MaxSpeed",          &Steering::SteeringComponent::MaxSpeed)
        .Field("TurnRateIdle",      &Steering::SteeringComponent::TurnRateIdle)
        .Field("TurnRateMoving",    &Steering::SteeringComponent::TurnRateMoving)
        .Field("AccelerationTime",  &Steering::SteeringComponent::AccelerationTime)
        .Field("DecelerationTime",  &Steering::SteeringComponent::DecelerationTime)
        .Field("AvoidanceRadius",   &Steering::SteeringComponent::AvoidanceRadius)
        .Field("SeparationDelay",   &Steering::SteeringComponent::SeparationDelay)
        .Field("SeparationRadius",  &Steering::SteeringComponent::SeparationRadius)
        .Field("SeparationStrength",&Steering::SteeringComponent::SeparationStrength);
}