
#pragma once

#include "PhoenixSteering/DLLExport.h"
#include "PhoenixSim/ECS/System.h"

namespace Phoenix::Steering
{
    class PHOENIX_STEERING_API SteeringSystem : public ECS::ISystem
    {
    public:
        PHX_DECLARE_TYPE(SteeringSystem, Phoenix::ECS::ISystem)

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
