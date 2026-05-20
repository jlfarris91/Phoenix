#include "Phoenix.Sim/Services/ISessionService.h"

using namespace Phoenix;

Session* ISessionService::GetSession() const
{
    return Session.get();
}

void ISessionService::OnSessionLayout(const SessionLayoutContext&, BlockBufferConfigBuilder&)
{
}

void ISessionService::Initialize(const std::shared_ptr<Phoenix::Session>& session)
{
    PHX_ASSERT(session);
    Session = session;
}

void ISessionService::Shutdown()
{
    Session.reset();
}

void ISessionService::OnWorldLayout(const WorldLayoutContext&, BlockBufferConfigBuilder&)
{
}

void ISessionService::OnWorldInitialize(WorldRef)
{
}

void ISessionService::OnWorldShutdown(WorldRef)
{
}
