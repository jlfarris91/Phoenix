#include "PhoenixMain.h"

#include <memory>
#include <vector>

#include "Application/Application.h"
#include "Phoenix/Services/ServiceContainerBuilder.h"
#include "Application/AppModule.h"
#include "Application/AppModuleManager.h"
#include "Phoenix/Reflection/TypeRegistry.h"
#include "Sessions/SessionDriverService.h"

using namespace Phoenix;

static void DiscoverAppModules(std::vector<std::shared_ptr<IAppModule>>& outModules)
{
    for (auto* desc : TypeRegistry::GetAllDerivedFrom<IAppModule>())
    {
        if (auto module = std::static_pointer_cast<IAppModule>(desc->MakeShared()))
        {
            outModules.push_back(module);
        }
    }
}

int Phoenix::PhoenixMain(int, char**)
{
    ServiceContainerBuilder builder;

    // Register default services
    // TODO (jfarris): implement service modules so we can bundle these and let apps decide the defaults
    {
        builder.Register<SessionDriverService>().AsInterfaces();
    }

    std::vector<std::shared_ptr<IAppModule>> modules;
    DiscoverAppModules(modules);

    for (auto& module : modules)
    {
        module->Register(builder);
    }

    builder.Register<AppModuleManager>([modules](IServiceLocator&)
    {
        auto manager = std::make_shared<AppModuleManager>();
        for (auto& module : modules)
        {
            manager->RegisterModule(module);
        }
        return manager;
    }).AsInterfaces();

    Application::CtorArgs args{&builder};
    auto app = std::make_shared<Application>(args);
    app->Run();

    return 0;
}