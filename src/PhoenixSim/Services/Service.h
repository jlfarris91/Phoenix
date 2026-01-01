#pragma once

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Reflection.h"
#include "PhoenixSim/Worlds.h"

namespace Phoenix
{
    class Session;

    class PHOENIX_SIM_API IService : public TSharedAsThis<IService>
    {
        PHX_DECLARE_INTERFACE(IService)

    public:

        // Gets the session that this service belongs to.
        Session* GetSession() const;

        virtual void Initialize(const TSharedPtr<Session>& session);
        virtual void Shutdown();

        virtual void OnWorldInitialize(WorldRef world);
        virtual void OnWorldShutdown(WorldRef world);

    protected:

        TSharedPtr<Session> Session;
    };
}
