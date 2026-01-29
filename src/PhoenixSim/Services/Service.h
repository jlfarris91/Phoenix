#pragma once

#include "PhoenixSim/Reflection.h"
#include "PhoenixSim/WorldsFwd.h"

namespace Phoenix
{
    class Session;
    struct WorldLayoutContext;
    struct WorldLayoutBuilder;

    class PHOENIX_SIM_API IService : public TSharedAsThis<IService>
    {
        PHX_DECLARE_INTERFACE(IService)

    public:

        // Gets the session that this service belongs to.
        Session* GetSession() const;

        virtual void OnSessionLayout(const WorldLayoutContext& context, WorldLayoutBuilder& builder);
        virtual void Initialize(const TSharedPtr<Session>& session);
        virtual void Shutdown();

        virtual void OnWorldLayout(const WorldLayoutContext& context, WorldLayoutBuilder& builder);
        virtual void OnWorldInitialize(WorldRef world);
        virtual void OnWorldShutdown(WorldRef world);

    protected:

        TSharedPtr<Session> Session;
    };
}
