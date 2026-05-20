#include "AppService.h"

void Phoenix::IAppService::Initialize(const std::shared_ptr<Application>& application)
{
    WeakApp = application;
}

void Phoenix::IAppService::Shutdown()
{
    WeakApp.reset();
}

void Phoenix::IAppService::PreTick() {}
void Phoenix::IAppService::Tick() {}
void Phoenix::IAppService::PostTick() {}
