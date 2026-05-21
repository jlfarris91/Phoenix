#include "PhoenixRTS2Module.h"

#include "SDL3ImGuiService.h"
#include "SDL3PlatformService.h"
#include "Application/Application.h"
#include "Logger/Logger.h"
#include "Phoenix/Parallel.h"
#include "Profiler/PhoenixTracyImpl.h"
#include "Phoenix/Profiling.h"
#include "Phoenix/Services/ServiceContainerBuilder.h"

using namespace Phoenix;

// ===== Profiling =====
TracyProfiler GTracyProfiler;

// ===== Logging =====
std::shared_ptr<Logger> GLogger;
bool GShowConsoleWindow = true;

void PhoenixRTS2Module::Register(ServiceContainerBuilder& builder)
{
    builder.Register<SDL3PlatformService>().AsInterfaces();
    builder.Register<UI::SDL3ImGuiService>().AsInterfaces();
}

void PhoenixRTS2Module::Initialize(Phoenix::ModuleInitContext &context)
{
    IAppModule::Initialize(context);

    auto app = GetApplication();

#ifndef __EMSCRIPTEN__
    Profiling::SetProfiler(&GTracyProfiler);

    unsigned int numThreads = std::min(std::thread::hardware_concurrency(), 8u);
    if (numThreads > 1)
    {
        SetThreadPool("SimThreadPool", numThreads - 1, 1024);
    }
#endif
}

void PhoenixRTS2Module::Load(Phoenix::ModuleLoadContext &context)
{
    IAppModule::Load(context);
}
