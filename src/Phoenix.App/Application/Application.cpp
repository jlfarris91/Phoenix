#include "Application.h"

#include <cassert>
#include <memory>

#include "AppService.h"
#include "Phoenix/Flags.h"
#include "Phoenix/Services/ServiceContainerBuilder.h"

using namespace Phoenix;

Application::Application(const CtorArgs& args)
{
    Container = args.Builder->Build();
}

void Application::Initialize()
{
    if (IsInitialized() || IsInitializing())
    {
        assert(!IsInitialized() && !IsInitializing());
        return;
    }

    if (IsShutDown() || IsShuttingDown())
    {
        assert(!IsShutDown() && !IsShuttingDown());
        return;
    }
    
    SetFlagRef(StateFlags, EAppStateFlags::Initializing);

    InitializeInternal();

    ClearFlagRef(StateFlags, EAppStateFlags::Initializing);
    SetFlagRef(StateFlags, EAppStateFlags::Initialized);
}

void Application::Shutdown()
{
    if (IsShutDown() || IsShuttingDown())
    {
        return;
    }
    
    SetFlagRef(StateFlags, EAppStateFlags::ShuttingDown);

    ShutdownInternal();

    ClearFlagRef(StateFlags, EAppStateFlags::ShuttingDown);
    SetFlagRef(StateFlags, EAppStateFlags::ShutDown);
}

void Application::Tick()
{
}

bool Application::IsInitializing() const
{
    return HasAnyFlags(StateFlags, EAppStateFlags::Initializing);
}

bool Application::IsInitialized() const
{
    return HasAnyFlags(StateFlags, EAppStateFlags::Initialized);
}

bool Application::IsShuttingDown() const
{
    return HasAnyFlags(StateFlags, EAppStateFlags::ShuttingDown);
}

bool Application::IsShutDown() const
{
    return HasAnyFlags(StateFlags, EAppStateFlags::ShutDown);
}

void Application::InitializeInternal()
{
    std::vector<std::shared_ptr<IAppService>> services;
    Container->ResolveServices<IAppService>(services);

    for (const auto& service : services)
    {
        service->Initialize(shared_from_this());
    }
}

void Application::ShutdownInternal()
{
    std::vector<std::shared_ptr<IAppService>> services;
    Container->ResolveServices<IAppService>(services);

    for (const auto& service : services)
    {
        service->Shutdown();
    }
}
