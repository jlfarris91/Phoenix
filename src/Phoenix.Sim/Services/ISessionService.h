#pragma once

#include <memory>

#include "Phoenix/Services/IService.h"
#include "Phoenix.Sim/WorldsFwd.h"

namespace Phoenix
{
    class Session;
    struct SessionLayoutContext;
    struct WorldLayoutContext;
    struct BlockBufferConfigBuilder;

    class PHOENIX_SIM_API ISessionService : public Phoenix::IService
    {
        PHX_DECLARE_TYPE_DERIVED(ISessionService, Phoenix::IService)

    public:
        virtual ~ISessionService() = default;

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
