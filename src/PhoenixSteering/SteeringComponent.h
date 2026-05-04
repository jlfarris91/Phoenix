
#pragma once

#include "PhoenixSteering/DLLExport.h"
#include "PhoenixSim/ECS/Component.h"
#include "PhoenixSim/ECS/EntityId.h"
#include "PhoenixSim/FixedPoint/FixedVector.h"
#include "PhoenixSim/Reflection/Registration.h"

namespace Phoenix::Steering
{
    enum class PHOENIX_STEERING_API ESteerMode
    {
        Idle,
        Move,
        Turn,
        Hold
    };

    enum class PHOENIX_STEERING_API ESteerFlags
    {
        None            = 0x0,
        Active          = 0x1,
        SeekingGoal     = 0x2,
        ArrivedAtGoal   = 0x4,
        FailedPathPlan  = 0x8,
        LockFacing      = 0x10,
        SteeringLeft    = 0x20,
        SteeringRight   = 0x40,
        Bumped          = 0x80,
        Followed        = 0x100,
        Attached        = 0x200,
        Hidden          = 0x400,
    };

    struct PHOENIX_STEERING_API SteeringComponent : ECS::IComponent
    {
        PHX_DECLARE_TYPE(SteeringComponent, Phoenix::ECS::IComponent)

        ESteerMode Mode = ESteerMode::Idle;
        ESteerFlags Flags = ESteerFlags::Active;

        uint32 CollisionMask = 0;
        uint32 Team = 0;

        ECS::EntityId GoalEntity;
        Vec2 GoalPos;

        Vec2 StepPos[2];

        Distance Slack;
        Distance SpreadingSlack;

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

        uint8 PushPriorityAlly = 0;
        uint8 PushPriorityGlobal = 0;
        uint8 PriorityPushed = 0;
    };
}

PHX_DEFINE_ENUM(Phoenix::Steering::ESteerMode)
{
    using namespace Phoenix::Steering;
    registration
        .Value("Idle", ESteerMode::Idle)
        .Value("Move", ESteerMode::Move)
        .Value("Turn", ESteerMode::Turn)
        .Value("Hold", ESteerMode::Hold);
}

PHX_DEFINE_ENUM(Phoenix::Steering::ESteerFlags)
{
    using namespace Phoenix::Steering;
    registration
        .EnumFlags()
        .Value("None",              ESteerFlags::None)
        .Value("Active",            ESteerFlags::Active)
        .Value("SeekingGoal",       ESteerFlags::SeekingGoal)
        .Value("ArrivedAtGoal",     ESteerFlags::ArrivedAtGoal)
        .Value("FailedPathPlan",    ESteerFlags::FailedPathPlan)
        .Value("LockFacing",        ESteerFlags::LockFacing)
        .Value("SteeringLeft",      ESteerFlags::SteeringLeft)
        .Value("SteeringRight",     ESteerFlags::SteeringRight)
        .Value("Bumped",            ESteerFlags::Bumped)
        .Value("Followed",          ESteerFlags::Followed)
        .Value("Attached",          ESteerFlags::Attached)
        .Value("Hidden",            ESteerFlags::Hidden);
}

PHX_DEFINE_TYPE(Phoenix::Steering::SteeringComponent)
{
    using namespace Phoenix::Steering;
    registration
        .Field("Mode",              &SteeringComponent::Mode)
        .Field("Flags",             &SteeringComponent::Flags)
        .Field("CollisionMask",     &SteeringComponent::CollisionMask)
        .Field("Team",              &SteeringComponent::Team)
        .Field("GoalEntity",        &SteeringComponent::GoalEntity)
        .Field("GoalPos",           &SteeringComponent::GoalPos)
        .Field("Slack",             &SteeringComponent::Slack)
        .Field("SpreadingSlack",    &SteeringComponent::SpreadingSlack)
        .Field("Velocity",          &SteeringComponent::Velocity)
        .Field("PreviousPos",       &SteeringComponent::PreviousPos)
        .Field("BestPos",           &SteeringComponent::BestPos)
        .Field("InnerRadius",       &SteeringComponent::InnerRadius)
        .Field("OuterRadius",       &SteeringComponent::OuterRadius)
        .Field("ArrivalRange",      &SteeringComponent::ArrivalRange)
        .Field("MaxSpeed",          &SteeringComponent::MaxSpeed)
        .Field("TurnRateIdle",      &SteeringComponent::TurnRateIdle)
        .Field("TurnRateMoving",    &SteeringComponent::TurnRateMoving)
        .Field("AccelerationTime",  &SteeringComponent::AccelerationTime)
        .Field("DecelerationTime",  &SteeringComponent::DecelerationTime)
        .Field("AvoidanceRadius",   &SteeringComponent::AvoidanceRadius)
        .Field("SeparationDelay",   &SteeringComponent::SeparationDelay)
        .Field("SeparationRadius",  &SteeringComponent::SeparationRadius)
        .Field("SeparationStrength",&SteeringComponent::SeparationStrength);
}