#include "Phoenix.Sim/Services/Service.h"

using namespace Phoenix;

Session* IService::GetSession() const
{
    return Session.get();
}

void IService::OnSessionLayout(const SessionLayoutContext& context, BlockBufferConfigBuilder& builder)
{
}

void IService::Initialize(const std::shared_ptr<Phoenix::Session>& session)
{
    PHX_ASSERT(session);
    Session = session;
}

void IService::Shutdown()
{
    Session.reset();
}

void IService::OnWorldLayout(const WorldLayoutContext& context, BlockBufferConfigBuilder& builder)
{
}

void IService::OnWorldInitialize(WorldRef world)
{
}

void IService::OnWorldShutdown(WorldRef world)
{
}
