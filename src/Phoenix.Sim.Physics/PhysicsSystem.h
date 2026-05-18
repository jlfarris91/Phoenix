
#pragma once

#include "PhoenixPhysics/DLLExport.h"
#include "PhoenixSim/ECS/JobScheduler.h"
#include "PhoenixSim/ECS/System.h"

namespace Phoenix
{
    class IDebugRenderer;
    class IDebugState;
}

namespace Phoenix::Physics
{
    // Forward-declared job types so the header stays light.
    namespace PhysicsSystemDetail
    {
        struct PopulateSortedEntitiesJob;
        struct SortEntitiesByZCodeTask;
        struct IntegrateVelocitiesJob;
        struct CalculateContactPairsJob;
        struct IntegrateJob;
    }

    class PHOENIX_PHYSICS_API PhysicsSystem : public ECS::ISystem
    {
    public:
        PHX_DECLARE_TYPE_DERIVED(PhysicsSystem, ISystem)

        PhysicsSystem();
        ~PhysicsSystem() override;

        void OnWorldInitialize(WorldRef world) override;
        void OnPreWorldUpdate(WorldRef world, const ECS::SystemUpdateArgs& args) override;
        void OnWorldUpdate(WorldRef world, const ECS::SystemUpdateArgs& args) override;
        void OnPostWorldUpdate(WorldRef world, const ECS::SystemUpdateArgs& args) override;

        void OnDebugRender(WorldConstRef world, const IDebugState& state, IDebugRenderer& renderer) override;

        bool DebugDrawContacts = false;
        bool AllowSleep = true;
        uint8 NumIterations = 2;
        uint8 NumSolverSteps = 6;
        uint8 NumSeparationSteps = 40;
        double PenetrationThreshold = 0.05;
        double PenetrationCorrection = 0.1;

    private:

        // Post-update (before loop): integrate velocities once.
        ECS::JobScheduler IntegrateVelocitiesScheduler;
        std::unique_ptr<PhysicsSystemDetail::IntegrateVelocitiesJob> IntegrateVelocitiesJob;

        // Per-iteration: calculate contact pairs.
        ECS::JobScheduler CalculateContactPairsScheduler;
        std::unique_ptr<PhysicsSystemDetail::CalculateContactPairsJob> CalculateContactPairsJob;

        // Per-iteration: integrate positions.
        ECS::JobScheduler IntegrateScheduler;
        std::unique_ptr<PhysicsSystemDetail::IntegrateJob> IntegrateJob;
    };
}

PHX_DEFINE_TYPE(Phoenix::Physics::PhysicsSystem)
{
    registration
        .ScriptHidden()
        .Field("DebugDrawContacts",     &Physics::PhysicsSystem::DebugDrawContacts)
        .Field("AllowSleep",            &Physics::PhysicsSystem::AllowSleep)
        .Field("NumIterations",         &Physics::PhysicsSystem::NumIterations)
        .Field("NumSolverSteps",        &Physics::PhysicsSystem::NumSolverSteps)
        .Field("NumSeparationSteps",    &Physics::PhysicsSystem::NumSeparationSteps)
        .Field("PenetrationThreshold",  &Physics::PhysicsSystem::PenetrationThreshold)
        .Field("PenetrationCorrection", &Physics::PhysicsSystem::PenetrationCorrection);
}