#include "WorldSceneSync.h"

#include <unordered_set>

#include <Phoenix.Sim.ECS/FeatureECS.h>
#include <Phoenix.Sim.ECS/FixedEntityList.h>

using namespace Phoenix::Scene;

void WorldSceneSync::Sync(WorldConstRef world)
{
    const ECS::FixedEntityList* entities = ECS::FeatureECS::GetEntities(world);
    if (!entities)
    {
        return;
    }

    std::unordered_set<ECS::EntityId> current;
    current.reserve(entities->GetNumActive());
    entities->ForEach([&](const ECS::Entity& entity)
    {
        current.insert(entity.Id);
    });

    for (auto it = EntityState.begin(); it != EntityState.end(); )
    {
        if (!current.contains(it->first))
        {
            OnDestroyEntity.Execute(world, it->second);
            it = EntityState.erase(it);
        }
        else
        {
            ++it;
        }
    }

    entities->ForEach([&](const ECS::Entity& entity)
    {
        if (!EntityState.contains(entity.Id))
        {
            SyncEntry syncEntry = { {}, entity.Id, entity.Kind };
            syncEntry.SceneEntity = OnSpawnEntity.Execute(world, syncEntry);
            if (!syncEntry.SceneEntity.IsValid())
            {
                return;
            }
            EntityState[entity.Id] = syncEntry;
        }

        OnUpdateEntity.Execute(world, EntityState[entity.Id]);
    });
}
