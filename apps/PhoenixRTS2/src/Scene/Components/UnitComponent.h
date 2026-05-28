#pragma once

#include <cstdint>

#include "Phoenix/Name.h"

namespace Phoenix::EnTT
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

    void OnSpawn(const Phoenix::EnTT::SceneComponentSyncArgs& args);
    void OnUpdate(const Phoenix::EnTT::SceneComponentSyncArgs& args);
    void OnDestroy(const Phoenix::EnTT::SceneComponentSyncArgs& args);

private:

    uint8_t OwningPlayer = 0;
    Phoenix::FName UnitData;
};
