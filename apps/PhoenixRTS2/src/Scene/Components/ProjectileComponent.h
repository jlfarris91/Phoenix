#pragma once

#include "ResourcePtr.h"
#include "Scene.h"
#include "Resources/LineMesh2D.h"

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
    Phoenix::FName ProjectileData;
    Phoenix::Renderer::TResourcePtr<Phoenix::App::Dev::LineMesh2D> Mesh;
    float Scale = 1.0f;
    glm::vec4 Tint = glm::vec4(1.0f);
};
