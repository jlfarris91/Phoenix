#include "PhoenixSim/Services/Service.h"

using namespace Phoenix;

Session* IService::GetSession() const
{
    return Session.get();
}

void IService::OnSessionLayout(const WorldLayoutContext& context, WorldLayoutBuilder& builder)
{
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

void IService::OnWorldLayout(const WorldLayoutContext& context, WorldLayoutBuilder& builder)
{
}

void IService::OnWorldInitialize(WorldRef world)
{
}

void IService::OnWorldShutdown(WorldRef world)
{
}
