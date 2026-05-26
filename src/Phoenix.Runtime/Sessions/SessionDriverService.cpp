#include "SessionDriverService.h"

using namespace Phoenix;

void SessionDriverService::Initialize(const std::shared_ptr<Application> &application)
{
    IAppService::Initialize(application);
    SessionDriver = std::make_unique<Phoenix::SessionDriver>();

    auto thisSP = std::static_pointer_cast<SessionDriverService>(shared_from_this());
    SessionDriver->SessionCreated.AddSP(thisSP, &SessionDriverService::OnSessionCreated);
    SessionDriver->SessionDestroyed.AddSP(thisSP, &SessionDriverService::OnSessionDestroyed);
}

void SessionDriverService::Shutdown()
{
    SessionDriver.reset();
    IAppService::Shutdown();
}

SessionDriver& SessionDriverService::GetSessionDriver() const
{
    return *SessionDriver.get();
}

void SessionDriverService::OnSessionCreated(SessionInstance *instance)
{
    SessionCreated.Broadcast(instance);
}

void SessionDriverService::OnSessionDestroyed(SessionInstance *instance)
{
    SessionDestroyed.Broadcast(instance);
}
