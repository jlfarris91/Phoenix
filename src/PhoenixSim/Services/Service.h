#pragma once

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Reflection.h"

namespace Phoenix
{
    class Session;

    class PHOENIX_SIM_API IService : TSharedAsThis<IService>
    {
        PHX_DECLARE_TYPE(IService)

    public:

        // Gets the session that this service belongs to.
        Session* GetSession() const;

        virtual void Initialize(const TSharedPtr<Session>& session);
        virtual void Shutdown();

    protected:

        TSharedPtr<Session> Session;
    };
}
