#include "Scene.h"

#include "SceneComponentHandler.h"
#include "Phoenix.Sim.ECS/FeatureECS.h"
#include "Phoenix/Logging.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::Scene;
using namespace Phoenix::EnTT;

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

EnTT::Scene::Scene(FName id)
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

entt::registry & EnTT::Scene::GetRegistry()
{
    return Registry;
}

const entt::registry & EnTT::Scene::GetRegistry() const
{
    return Registry;
}

void EnTT::Scene::Sync(WorldConstRef world)
{
    EntitySync.Sync(world);
}

Renderer::ISceneProxyManager& EnTT::Scene::GetProxyManager() const
{
    return *ProxyManager;
}

void EnTT::Scene::RegisterSyncComponentHandler(const std::shared_ptr<ISceneComponentHandler>& handler)
{
    auto iter = std::ranges::find(SceneComponentHandlers, handler);
    if (iter != SceneComponentHandlers.end())
    {
        return;
    }
    SceneComponentHandlers.push_back(handler);
    handler->Register(shared_from_this());
}

bool EnTT::Scene::UnregisterSyncComponentHandler(const std::shared_ptr<ISceneComponentHandler>& handler)
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

SceneEntity EnTT::Scene::OnSpawnEntity(WorldConstRef world, const WorldSceneSync::SyncEntry& entry)
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

    LogVerbose("Spawning scene entity for sim entity {}", (uint32_t)entry.SimEntity);

    return Pack(sceneEntity);
}

void EnTT::Scene::OnUpdateEntity(WorldConstRef world, const WorldSceneSync::SyncEntry& entry)
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

void EnTT::Scene::OnDestroyEntity(WorldConstRef world, const WorldSceneSync::SyncEntry& entry)
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
                handler->OnDestroyComponent(compArgs);
                break;
            }
        }
    });

    LogVerbose("Destroyed scene entity for sim entity {}", (uint32_t)entry.SimEntity);
}