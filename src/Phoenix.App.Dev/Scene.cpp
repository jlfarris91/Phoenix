#include "Scene.h"

#include "SceneComponentHandler.h"
#include "Phoenix.Sim.ECS/FeatureECS.h"
#include "Phoenix/Logging.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::Scene;
using namespace Phoenix::App::Dev;

namespace
{
    SceneEntity Pack(entt::entity e)
    {
        return { static_cast<uint64_t>(entt::to_integral(e)) };
    }

    entt::entity Unpack(SceneEntity e)
    {
        return static_cast<entt::entity>(e.Id);
    }
}

App::Dev::Scene::Scene(FName id)
    : Id(id)
{
    EntitySync.OnSpawnEntity.BindLambda([this](WorldConstRef world, const WorldSceneSync::SyncEntry& entry)
    {
        return this->OnSpawnEntity(world, entry);
    });
    EntitySync.OnUpdateEntity.BindLambda([this](WorldConstRef world, const WorldSceneSync::SyncEntry& entry)
    {
        this->OnUpdateEntity(world, entry);
    });
    EntitySync.OnDestroyEntity.BindLambda([this](WorldConstRef world, const WorldSceneSync::SyncEntry& entry)
    {
        this->OnDestroyEntity(world, entry);
    });
}

entt::registry & App::Dev::Scene::GetRegistry()
{
    return Registry;
}

const entt::registry & App::Dev::Scene::GetRegistry() const
{
    return Registry;
}

void App::Dev::Scene::Sync(WorldConstRef world)
{
    EntitySync.Sync(world);
}

void App::Dev::Scene::RegisterSyncComponentHandler(const std::shared_ptr<ISceneComponentHandler>& handler)
{
    auto iter = std::ranges::find(SceneComponentHandlers, handler);
    if (iter != SceneComponentHandlers.end())
    {
        return;
    }
    SceneComponentHandlers.push_back(handler);
    handler->Register(shared_from_this());
}

bool App::Dev::Scene::UnregisterSyncComponentHandler(const std::shared_ptr<ISceneComponentHandler>& handler)
{
    auto iter = std::ranges::find(SceneComponentHandlers, handler);
    if (iter == SceneComponentHandlers.end())
    {
        return false;
    }
    SceneComponentHandlers.erase(iter);
    handler->Unregister();
    return true;
}

SceneEntity App::Dev::Scene::OnSpawnEntity(WorldConstRef world, const WorldSceneSync::SyncEntry& entry)
{
    auto sceneEntity = Registry.create();

    SceneComponentSyncArgs compArgs =
    {
        .World = &world,
        .Scene = this,
        .SceneEntity = sceneEntity,
        .SimEntity = entry.SimEntity
    };

    FeatureECS::ForEachComponent(world, entry.SimEntity, [&](const ComponentDefinition& compDef, const void* comp)
    {
        compArgs.SimComponentTypeId = compDef.TypeDescriptor->GetTypeId();
        compArgs.SimComponentData = comp;
        for (auto&& handler : SceneComponentHandlers)
        {
            if (handler->CanSync(compArgs))
            {
                handler->OnSync(compArgs);
                break;
            }
        }
    });

    LogVerbose("Spawning scene entity for sim entity {} of kind {}", (uint32_t)entry.SimEntity, entry.Kind.ToString());

    return Pack(sceneEntity);
}

void App::Dev::Scene::OnUpdateEntity(WorldConstRef world, const WorldSceneSync::SyncEntry& entry)
{
    SceneComponentSyncArgs compArgs =
    {
        .World = &world,
        .Scene = this,
        .SceneEntity = Unpack(entry.SceneEntity),
        .SimEntity = entry.SimEntity
    };

    FeatureECS::ForEachComponent(world, entry.SimEntity, [&](const ComponentDefinition& compDef, const void* comp)
    {
        compArgs.SimComponentTypeId = compDef.TypeDescriptor->GetTypeId();
        compArgs.SimComponentData = comp;
        for (auto&& handler : SceneComponentHandlers)
        {
            if (handler->CanSync(compArgs))
            {
                handler->OnSync(compArgs);
                break;
            }
        }
    });
}

void App::Dev::Scene::OnDestroyEntity(WorldConstRef, const WorldSceneSync::SyncEntry& entry)
{
    Registry.destroy(Unpack(entry.SceneEntity));
    LogVerbose("Destroyed scene entity for sim entity {} of kind {}", (uint32_t)entry.SimEntity, entry.Kind.ToString());
}