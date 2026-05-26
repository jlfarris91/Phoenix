#pragma once

#include <cstdint>

#include "Phoenix/Name.h"

namespace Phoenix::EnTT
{
    struct SceneComponentHandlerArgs;
}

namespace Phoenix::ECS
{
    struct EntityId;
}

class UnitComponent
{
public:

    void OnSpawn(const Phoenix::EnTT::SceneComponentHandlerArgs& args);
    void OnUpdate(const Phoenix::EnTT::SceneComponentHandlerArgs& args);
    void OnDestroy(const Phoenix::EnTT::SceneComponentHandlerArgs& args);

private:

    uint8_t OwningPlayer = 0;
    Phoenix::FName UnitData;
};
