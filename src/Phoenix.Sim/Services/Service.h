#pragma once

#include "PhoenixSim/Reflection/Registration.h"
#include "PhoenixSim/WorldsFwd.h"

namespace Phoenix
{
    class Session;
    struct SessionLayoutContext;
    struct WorldLayoutContext;
    struct BlockBufferConfigBuilder;

    class PHOENIX_SIM_API IService : public std::enable_shared_from_this<IService>
    {
        PHX_DECLARE_TYPE_INTERFACE(IService)

    public:
        virtual ~IService() = default;

        // Gets the session that this service belongs to.
        Session* GetSession() const;

        virtual void OnSessionLayout(const SessionLayoutContext& context, BlockBufferConfigBuilder& builder);
        virtual void Initialize(const std::shared_ptr<Session>& session);
        virtual void Shutdown();

        virtual void OnWorldLayout(const WorldLayoutContext& context, BlockBufferConfigBuilder& builder);
        virtual void OnWorldInitialize(WorldRef world);
        virtual void OnWorldShutdown(WorldRef world);

    protected:

        std::shared_ptr<Session> Session;
    };
}
