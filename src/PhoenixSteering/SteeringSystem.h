
#pragma once

#include "PhoenixSteering/DLLExport.h"
#include "PhoenixSim/ECS/JobScheduler.h"
#include "PhoenixSim/ECS/System.h"

namespace Phoenix::Steering
{
    namespace SteeringDetail
    {
        struct PopulateSortedEntitiesJob;
        struct SortEntitiesByZCodeTask;
        struct PathfindingJob;
        struct SteeringJob;
        struct CollisionJob;
    }

    class PHOENIX_STEERING_API SteeringSystem : public ECS::ISystem
    {
    public:
        PHX_DECLARE_TYPE_DERIVED(SteeringSystem, Phoenix::ECS::ISystem)

        void OnWorldInitialize(WorldRef world) override;
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

    private:
        // Pre-update: populate sorted entities + sort by z-code.
        ECS::JobScheduler PreUpdateScheduler;

        // Update: pathfinding → steering → collision × 2.
        ECS::JobScheduler UpdateScheduler;
        SteeringDetail::PathfindingJob* PathfindingJobPtr  = nullptr;
        SteeringDetail::SteeringJob*    SteeringJobPtr     = nullptr;
        SteeringDetail::CollisionJob*   CollisionJobPtr[2] = {};
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