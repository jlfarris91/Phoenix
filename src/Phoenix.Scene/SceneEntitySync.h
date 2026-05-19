#pragma once

#include <memory>
#include <unordered_map>

#include <Phoenix/Name.h>
#include <Phoenix.Sim/WorldsFwd.h>
#include <Phoenix.Sim.ECS/EntityId.h>

#include "EngineEntity.h"
#include "IEntitySyncHandler.h"

namespace Phoenix
{
    class SceneEntitySync
    {
    public:
        void RegisterHandler(FName kind, std::shared_ptr<IEntitySyncHandler> handler);

        // Sync sim entities from a specific world into engine entities.
        // Safe to call with different worlds across frames — state is tracked per-world.
        void Sync(WorldConstRef world);

        // Remove tracking state for a world (call when a world is destroyed).
        void RemoveWorld(WorldConstRef world);

    private:
        struct SyncEntry
        {
            EngineEntity Entity;
            FName        Kind;
        };

        using WorldSyncMap = std::unordered_map<ECS::EntityId, SyncEntry>;

        std::unordered_map<FName, std::shared_ptr<IEntitySyncHandler>> Handlers;
        std::unordered_map<const World*, WorldSyncMap>                  WorldStates;
    };
}
