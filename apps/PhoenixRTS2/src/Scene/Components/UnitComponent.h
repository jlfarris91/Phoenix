#pragma once

#include <cstdint>

#include "Phoenix/Name.h"

namespace Phoenix::App::Dev
{
    struct SceneComponentSyncArgs;
}

namespace Phoenix::ECS
{
    struct EntityId;
}

class UnitComponent
{
public:

    void OnSpawn(const Phoenix::App::Dev::SceneComponentSyncArgs& args);
    void OnUpdate(const Phoenix::App::Dev::SceneComponentSyncArgs& args);
    void OnDestroy(const Phoenix::App::Dev::SceneComponentSyncArgs& args);

private:

    uint8_t OwningPlayer = 0;
    Phoenix::FName UnitData;
};
