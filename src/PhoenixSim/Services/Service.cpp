#include "PhoenixSim/Services/Service.h"

using namespace Phoenix;

Session* IService::GetSession() const
{
    return Session.get();
}

void IService::Initialize(const TSharedPtr<Phoenix::Session>& session)
{
    PHX_ASSERT(session);
    Session = session;
}

void IService::Shutdown()
{
    Session.reset();
}

void IService::OnWorldInitialize(WorldRef world)
{
}

void IService::OnWorldShutdown(WorldRef world)
{
}
