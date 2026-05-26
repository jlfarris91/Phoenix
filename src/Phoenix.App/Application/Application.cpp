#include "Application.h"

#include <cassert>
#include <memory>
#include <mutex>
#include <shared_mutex>

#include "AppModuleManager.h"
#include "AppService.h"
#include "IPlatformService.h"
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

void Application::Run()
{
    Initialize();
    while (!WantsQuit())
    {
        ExecuteDispatchQueue();
        Tick();
    }
    Shutdown();
}

void Application::RequestQuit()
{
    SetFlagRef(StateFlags, EAppStateFlags::QuitRequested);
}

bool Application::WantsQuit() const
{
    if (HasAnyFlags(StateFlags, EAppStateFlags::QuitRequested))
    {
        return true;
    }
    if (auto platform = GetService<IPlatformService>())
    {
        return platform->WantsQuit();
    }
    return false;
}

void Application::Tick()
{
    for (const auto& service : AppServices)
    {
        service->PreTick();
    }

    for (const auto& service : AppServices)
    {
        service->Tick();
    }

    for (const auto& service : AppServices)
    {
        service->PostTick();
    }
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

void Application::Dispatch(std::function<void()>&& function)
{
    std::scoped_lock lock(DispatchQueueMutex);
    DispatchQueue.emplace(std::move(function));
}

void Application::InitializeInternal()
{
    Container->ResolveServices<IAppService>(AppServices);

    for (const auto& service : AppServices)
    {
        service->Initialize(shared_from_this());
    }

    if (auto&& moduleManager = Container->ResolveService<AppModuleManager>())
    {
        for (auto&& module : moduleManager->GetModules())
        {
            ModuleLoadContext context;
            module->Load(context);
        }
    }
}

void Application::ShutdownInternal()
{
    for (const auto& service : AppServices)
    {
        service->Shutdown();
    }
    AppServices.clear();
}

void Application::ExecuteDispatchQueue()
{
    std::scoped_lock lock(DispatchQueueMutex);

    size_t count = DispatchQueue.size();
    for (size_t i = 0; i < count; i++)
    {
        const auto& func = DispatchQueue.front();
        func();
        DispatchQueue.pop();
    }
}
