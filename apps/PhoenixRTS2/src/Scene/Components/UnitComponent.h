#pragma once

#include <cstdint>

#include "Scene.h"
#include "Phoenix/Name.h"

namespace Phoenix::App::Dev
{
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
};
