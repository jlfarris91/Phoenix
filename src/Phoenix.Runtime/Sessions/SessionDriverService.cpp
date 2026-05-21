#include "SessionDriverService.h"

using namespace Phoenix;

void SessionDriverService::Initialize(const std::shared_ptr<Application> &application)
{
    IAppService::Initialize(application);
    SessionDriver = std::make_unique<Phoenix::SessionDriver>();
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
