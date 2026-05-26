#include "SceneModule.h"

#include "AutoSyncComponentHandler.h"
#include "Scene.h"
#include "SceneManager.h"
#include "Application/Application.h"
#include "Components/UnitComponent.h"
#include "Phoenix.Sim.RTS/Units/UnitComponent.h"
#include "Sessions/SessionDriverService.h"
#include "Worlds/WorldInstance.h"

void SceneModule::Register(Phoenix::ServiceContainerBuilder &builder)
{
    IAppModule::Register(builder);

    builder.Register<Phoenix::EnTT::SceneManager>().AsInterfaces();
}

void SceneModule::Initialize(Phoenix::ModuleInitContext &context)
{
    IAppModule::Initialize(context);

    auto app = GetApplication();

    SceneManager = app->ResolveService<Phoenix::Scene::ISceneManager>();

    auto thisSP = std::static_pointer_cast<SceneModule>(shared_from_this());

    auto sessionDriver = app->GetService<Phoenix::SessionDriverService>();
    sessionDriver->SessionCreated.AddSP(thisSP, &SceneModule::OnSessionInstanceCreated);
    sessionDriver->SessionDestroyed.AddSP(thisSP, &SceneModule::OnSessionInstanceDestroyed);
}

void SceneModule::Shutdown()
{
    auto app = GetApplication();

    if (auto sessionDriver = app->GetService<Phoenix::SessionDriverService>())
    {
        sessionDriver->SessionCreated.RemoveAll(this);
        sessionDriver->SessionDestroyed.RemoveAll(this);
    }

    IAppModule::Shutdown();
}

void SceneModule::OnSessionInstanceCreated(Phoenix::SessionInstance* instance)
{
    auto thisSP = std::static_pointer_cast<SceneModule>(shared_from_this());
    instance->WorldInstanceCreated.AddSP(thisSP, &SceneModule::OnWorldInstanceCreated);
    instance->WorldInstanceUpdated.AddSP(thisSP, &SceneModule::OnWorldInstanceUpdated);
    instance->WorldInstanceDestroyed.AddSP(thisSP, &SceneModule::OnWorldInstanceDestroyed);
}

void SceneModule::OnSessionInstanceDestroyed(Phoenix::SessionInstance* instance)
{
    instance->WorldInstanceCreated.RemoveAll(this);
    instance->WorldInstanceUpdated.RemoveAll(this);
    instance->WorldInstanceDestroyed.RemoveAll(this);
}

void SceneModule::OnWorldInstanceCreated(Phoenix::WorldInstance* instance, Phoenix::WorldConstRef world)
{
    if (auto scene = SceneManager->CreateScene(instance->GetId()))
    {
        auto enttScene = std::static_pointer_cast<Phoenix::EnTT::Scene>(scene);
        RegisterSceneComponentHandlers(*enttScene);

        scene->Sync(world);
    }
}

void SceneModule::OnWorldInstanceUpdated(Phoenix::WorldInstance* instance, Phoenix::WorldConstRef world)
{
    if (auto scene = SceneManager->FindScene(instance->GetId()))
    {
        scene->Sync(world);
    }
}

void SceneModule::OnWorldInstanceDestroyed(Phoenix::WorldInstance* instance)
{
    SceneManager->DestroyScene(instance->GetId());
}

void SceneModule::RegisterSceneComponentHandlers(Phoenix::EnTT::Scene& scene)
{
    scene.RegisterSyncComponentHandler<Phoenix::EnTT::AutoSceneComponentHandler<Phoenix::RTS::UnitComponent, UnitComponent>>();
}
