#include "PhoenixSteering/SteeringComponent.h"
#include "PhoenixSim/Reflection/Registration.h"

using namespace Phoenix;
using namespace Phoenix::Steering;

PHX_DEFINE_TYPE(SteeringComponent)
{
    registration
        .Field("Mode",              &SteeringComponent::Mode)
        .Field("Flags",             &SteeringComponent::Flags)
        .Field("CollisionMask",     &SteeringComponent::CollisionMask)
        .Field("GoalEntity",        &SteeringComponent::GoalEntity)
        .Field("GoalPos",           &SteeringComponent::GoalPos)
        .Field("Slack",             &SteeringComponent::Slack)
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
