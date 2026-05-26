#pragma once

#include <unordered_map>

#include <Phoenix/Name.h>
#include <Phoenix.Sim/WorldsFwd.h>
#include <Phoenix.Sim.ECS/EntityId.h>

#include "SceneEntity.h"
#include "Phoenix/Delegates.h"

namespace Phoenix::Scene
{
    class IScene;

    class WorldSceneSync
    {
    public:
        // Sync sim entities from a specific world into engine entities.
        // Safe to call with different worlds across frames — state is tracked per-world.
        void Sync(WorldConstRef world);

        struct SyncEntry
        {
            SceneEntity     SceneEntity;
            ECS::EntityId   SimEntity;
            FName           Kind;
        };

        PHX_DECLARE_DELEGATE_RET(FOnSpawnEntity, SceneEntity, WorldConstRef, const SyncEntry&);
        FOnSpawnEntity OnSpawnEntity;

        PHX_DECLARE_DELEGATE(FOnUpdateEntity, WorldConstRef, const SyncEntry&);
        FOnUpdateEntity OnUpdateEntity;

        PHX_DECLARE_DELEGATE(FOnDestroyEntity, WorldConstRef, const SyncEntry&);
        FOnDestroyEntity OnDestroyEntity;

    private:

        std::unordered_map<ECS::EntityId, SyncEntry> EntityState;
    };
}
