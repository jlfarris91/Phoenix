
#pragma once

#include "PhoenixPhysics/DLLExport.h"
#include "PhoenixSim/ECS/System.h"

namespace Phoenix
{
    class IDebugRenderer;
    class IDebugState;
}

namespace Phoenix::Physics
{
    class PHOENIX_PHYSICS_API PhysicsSystem : public ECS::ISystem
    {
    public:
        PHX_DECLARE_TYPE(PhysicsSystem, Phoenix::ECS::ISystem)

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
    };
}
