#pragma once

#include "Scene.h"

namespace Phoenix::App::Dev
{
    struct SceneComponentSyncArgs;
}

class ProjectileComponent
{
public:
    void OnConstruct(Phoenix::App::Dev::Scene& scene, entt::entity entity);
    void OnDestroy(Phoenix::App::Dev::Scene& scene, entt::entity entity);
    void OnSync(const Phoenix::App::Dev::SceneComponentSyncArgs& args);
private:
    bool Foobar = false;
};
