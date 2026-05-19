#pragma once

#include <Phoenix.Sim/WorldsFwd.h>
#include <Phoenix.Sim.ECS/EntityId.h>

#include "EngineEntity.h"

namespace Phoenix
{
    class IEntitySyncHandler
    {
    public:
        virtual ~IEntitySyncHandler() = default;

        virtual EngineEntity OnSpawn  (WorldConstRef world, ECS::EntityId simEntity) = 0;
        virtual void         OnUpdate (EngineEntity engineEntity, WorldConstRef world, ECS::EntityId simEntity) = 0;
        virtual void         OnDespawn(EngineEntity engineEntity) = 0;
    };
}
