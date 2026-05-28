#include "AppService.h"

#include "Application.h"

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

std::thread::id Phoenix::IAppService::GetOwningThreadId() const
{
    auto app = GetApplication();
    PHX_ASSERT(app);
    return app ? app->GetOwningThreadId() : std::thread::id{};
}

bool Phoenix::IAppService::IsOnOwningThread() const
{
    auto app = GetApplication();
    PHX_ASSERT(app);
    return app && app->IsOnOwningThread();
}

void Phoenix::IAppService::Dispatch(std::function<void()>&& func)
{
    auto app = GetApplication();
    PHX_ASSERT(app);
    if (app)
    {
        app->Dispatch(std::move(func));
    }
}
