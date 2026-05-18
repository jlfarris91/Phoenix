
#pragma once

#include "Phoenix.Sim/Actions.h"
#include "Phoenix.Sim/Reflection/Registration.h"
#include "Phoenix.Sim/WorldsFwd.h"

namespace Phoenix
{
    class Session;
    class IDebugState;
    class IDebugRenderer;
}

namespace Phoenix::ECS
{
    enum class PHOENIX_SIM_API EJobPhase : uint8
    {
        PreUpdate,
        Update,
        PostUpdate,
    };

    struct PHOENIX_SIM_API SystemUpdateArgs
    {
        simtime_t SimTime = 0;
        DeltaTime DeltaTime;
    };

    struct PHOENIX_SIM_API SystemActionArgs
    {
        simtime_t SimTime = 0;
        Action Action;
    };

    class PHOENIX_SIM_API ISystem
    {
        PHX_DECLARE_TYPE_INTERFACE(ISystem)

    public:

        virtual ~ISystem() = default;

        virtual FName GetName() const { return FName::None; }

        virtual void OnPreUpdate(const SystemUpdateArgs& args) {}
        virtual void OnUpdate(const SystemUpdateArgs& args) {}
        virtual void OnPostUpdate(const SystemUpdateArgs& args) {}

        virtual bool OnPreHandleAction(const SystemActionArgs& args) { return false; }
        virtual bool OnHandleAction(const SystemActionArgs& args) { return false; }
        virtual bool OnPostHandleAction(const SystemActionArgs& args) { return false; }

        virtual void OnWorldInitialize(WorldRef world) {}
        virtual void OnWorldShutdown(WorldRef world) {}

        virtual void OnPreWorldUpdate(WorldRef world, const SystemUpdateArgs& args) {}
        virtual void OnWorldUpdate(WorldRef world,  const SystemUpdateArgs& args) {}
        virtual void OnPostWorldUpdate(WorldRef world, const SystemUpdateArgs& args) {}

        virtual bool OnPreHandleWorldAction(WorldRef world, const SystemActionArgs& args) { return false; }
        virtual bool OnHandleWorldAction(WorldRef world, const SystemActionArgs& args) { return false; }
        virtual bool OnPostHandleWorldAction(WorldRef world, const SystemActionArgs& args) { return false; }

        virtual void OnDebugRender(WorldConstRef world, const IDebugState& state, IDebugRenderer& renderer) {}

    protected:

        friend class FeatureECS;

        Session* Session = nullptr;
    };
}

#define PHX_ECS_DECLARE_SYSTEM(type) PHX_DECLARE_TYPE(type, Phoenix::ECS::ISystem)