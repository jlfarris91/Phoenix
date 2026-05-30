#pragma once

#include "Scene.h"
#include "Application/AppModule.h"
#include "Sessions/SessionInstance.h"

namespace Phoenix::Scene
{
    class ISceneManager;
    class IScene;
}

class SceneModule : public Phoenix::IAppModule
{
    PHX_DECLARE_TYPE_DERIVED(SceneModule, Phoenix::IAppModule);
public:

    void Register(Phoenix::ServiceContainerBuilder &builder) override;
    void Initialize(Phoenix::ModuleInitContext &context) override;
    void Shutdown() override;

private:

    void OnSessionInstanceCreated(Phoenix::SessionInstance* instance);
    void OnSessionInstanceDestroyed(Phoenix::SessionInstance* instance);

    void OnWorldInstanceCreated(Phoenix::WorldInstance* instance);
    void OnWorldInstanceUpdated(Phoenix::WorldInstance* instance);
    void OnWorldInstanceDestroyed(Phoenix::WorldInstance* instance);

    void RegisterSceneComponentHandlers(Phoenix::App::Dev::Scene& scene);

    std::shared_ptr<Phoenix::Scene::ISceneManager> SceneManager;
};
