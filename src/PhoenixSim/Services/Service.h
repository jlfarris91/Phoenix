#pragma once

#include "PhoenixSim/Reflection.h"
#include "PhoenixSim/WorldsFwd.h"

namespace Phoenix
{
    class Session;
    struct SessionLayoutContext;
    struct WorldLayoutContext;
    struct BlockBufferLayoutBuilder;

    class PHOENIX_SIM_API IService : public TSharedAsThis<IService>
    {
        PHX_DECLARE_INTERFACE(IService)

    public:

        // Gets the session that this service belongs to.
        Session* GetSession() const;

        virtual void OnSessionLayout(const SessionLayoutContext& context, BlockBufferLayoutBuilder& builder);
        virtual void Initialize(const TSharedPtr<Session>& session);
        virtual void Shutdown();

        virtual void OnWorldLayout(const WorldLayoutContext& context, BlockBufferLayoutBuilder& builder);
        virtual void OnWorldInitialize(WorldRef world);
        virtual void OnWorldShutdown(WorldRef world);

    protected:

        TSharedPtr<Session> Session;
    };
}
