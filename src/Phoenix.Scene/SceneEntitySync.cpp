#include "SceneEntitySync.h"

#include <unordered_set>

#include <Phoenix.Sim.ECS/FeatureECS.h>
#include <Phoenix.Sim.ECS/FixedEntityList.h>

namespace Phoenix
{
    void SceneEntitySync::RegisterHandler(FName kind, std::shared_ptr<IEntitySyncHandler> handler)
    {
        Handlers[kind] = std::move(handler);
    }

    void SceneEntitySync::Sync(WorldConstRef world)
    {
        const ECS::FixedEntityList* entities = ECS::FeatureECS::GetEntities(world);
        if (!entities)
            return;

        WorldSyncMap& simToEngine = WorldStates[&world];

        std::unordered_set<ECS::EntityId> current;
        entities->ForEach([&](const ECS::Entity& entity)
        {
            current.insert(entity.Id);
        });

        for (auto it = simToEngine.begin(); it != simToEngine.end(); )
        {
            if (!current.contains(it->first))
            {
                if (auto handlerIt = Handlers.find(it->second.Kind); handlerIt != Handlers.end())
                    handlerIt->second->OnDespawn(it->second.Entity);
                it = simToEngine.erase(it);
            }
            else
            {
                ++it;
            }
        }

        entities->ForEach([&](const ECS::Entity& entity)
        {
            auto handlerIt = Handlers.find(entity.Kind);
            if (handlerIt == Handlers.end())
                return;

            auto entryIt = simToEngine.find(entity.Id);
            if (entryIt == simToEngine.end())
            {
                EngineEntity engineEntity = handlerIt->second->OnSpawn(world, entity.Id);
                if (engineEntity.IsValid())
                    simToEngine[entity.Id] = { engineEntity, entity.Kind };
            }
            else
            {
                handlerIt->second->OnUpdate(entryIt->second.Entity, world, entity.Id);
            }
        });
    }

    void SceneEntitySync::RemoveWorld(WorldConstRef world)
    {
        auto it = WorldStates.find(&world);
        if (it == WorldStates.end())
            return;

        for (auto& [entityId, entry] : it->second)
        {
            if (auto handlerIt = Handlers.find(entry.Kind); handlerIt != Handlers.end())
                handlerIt->second->OnDespawn(entry.Entity);
        }

        WorldStates.erase(it);
    }
}
