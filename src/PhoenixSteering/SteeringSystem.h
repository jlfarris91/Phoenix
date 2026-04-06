
#pragma once

#include "PhoenixSteering/DLLExport.h"
#include "PhoenixSim/ECS/System.h"

namespace Phoenix::Steering
{
    class PHOENIX_STEERING_API SteeringSystem : public ECS::ISystem
    {
    public:
        PHX_DECLARE_TYPE_DERIVED(SteeringSystem, Phoenix::ECS::ISystem)

        void OnPreWorldUpdate(WorldRef world, const ECS::SystemUpdateArgs& args) override;
        void OnWorldUpdate(WorldRef world, const ECS::SystemUpdateArgs& args) override;

        void OnDebugRender(WorldConstRef world, const IDebugState& state, IDebugRenderer& renderer) override;

        bool MoveTowardsGoal = true;

        double DensityScalar = 0.3;
        double DensityRadiusScalar = 2.2;
        double AvoidanceScalar = 1.3;
        double AvoidanceRadiusScalar = 2.2;
        double ArrivalThreshold = 0.1;

        double SlackIncreaseRate = 1.0;
        double SlackIncreaseRateFast = 4.0;
        double SlackRateDivisor = 8.0;
        double SlackRateDivisorSlow = 32.0;
        double MaxSlack = 400.0;
    };
}

PHX_DEFINE_TYPE(Phoenix::Steering::SteeringSystem)
{
    registration
        .ScriptHidden()
        .Field("MoveTowardsGoal", &Steering::SteeringSystem::MoveTowardsGoal)
        .Field("DensityScalar", &Steering::SteeringSystem::DensityScalar)
        .Field("DensityRadiusScalar", &Steering::SteeringSystem::DensityRadiusScalar)
        .Field("AvoidanceScalar", &Steering::SteeringSystem::AvoidanceScalar)
        .Field("AvoidanceRadiusScalar", &Steering::SteeringSystem::AvoidanceRadiusScalar)
        .Field("ArrivalThreshold", &Steering::SteeringSystem::ArrivalThreshold)
        .Field("SlackIncreaseRate", &Steering::SteeringSystem::SlackIncreaseRate)
        .Field("SlackIncreaseRateFast", &Steering::SteeringSystem::SlackIncreaseRateFast)
        .Field("SlackRateDivisor", &Steering::SteeringSystem::SlackRateDivisor)
        .Field("SlackRateDivisorSlow", &Steering::SteeringSystem::SlackRateDivisorSlow)
        .Field("MaxSlack", &Steering::SteeringSystem::MaxSlack);
}