#pragma once

#include <cstdint>
#include <glm/glm.hpp>

#include "ResourcePtr.h"
#include "Scene.h"
#include "Phoenix/Name.h"

namespace Phoenix::App::Dev
{
    class LineMesh2D;
    struct SceneComponentSyncArgs;
}

class UnitComponent
{
public:

    void OnConstruct(Phoenix::App::Dev::Scene& scene, entt::entity entity);
    void OnDestroy(Phoenix::App::Dev::Scene& scene, entt::entity entity);
    void OnSync(const Phoenix::App::Dev::SceneComponentSyncArgs& args);

private:

    uint8_t OwningPlayer = 0;
    Phoenix::FName UnitData;
    bool bIsAlive = false;
    Phoenix::Renderer::TResourcePtr<Phoenix::App::Dev::LineMesh2D> Mesh;
    float Scale = 1.0f;
    glm::vec4 Tint = glm::vec4(1.0f);
};
