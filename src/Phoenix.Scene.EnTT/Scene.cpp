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

Renderer::RenderScene EnTT::Scene::Gather(WorldConstRef world) const
{
    Renderer::RenderScene scene;
    // scene.View   = Camera.GetView();
    // scene.Target = Target;
    return scene;
}

void EnTT::Scene::RegisterSyncComponentHandler(const std::shared_ptr<ISceneComponentHandler>& handler)
{
    SceneComponentHandlers.push_back(handler);
}

SceneEntity EnTT::Scene::OnSpawnEntity(WorldConstRef world, const WorldSceneSync::SyncEntry& entry)
{
    auto sceneEntity = Registry.create();

    SceneComponentHandlerArgs compArgs =
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
            if (handler->CanHandleSimComponent(compArgs))
            {
                handler->OnSpawnComponent(compArgs);
                break;
            }
        }
    });

    LogVerbose("Spawning scene entity for sim entity {}", (uint32_t)entry.SimEntity);

    return Pack(sceneEntity);
}

void EnTT::Scene::OnUpdateEntity(WorldConstRef world, const WorldSceneSync::SyncEntry& entry)
{
    SceneComponentHandlerArgs compArgs =
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
            if (handler->CanHandleSimComponent(compArgs))
            {
                handler->OnUpdateComponent(compArgs);
                break;
            }
        }
    });
}

void EnTT::Scene::OnDestroyEntity(WorldConstRef world, const WorldSceneSync::SyncEntry& entry)
{
    SceneComponentHandlerArgs compArgs =
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
            if (handler->CanHandleSimComponent(compArgs))
            {
                handler->OnDestroyComponent(compArgs);
                break;
            }
        }
    });

    LogVerbose("Destroyed scene entity for sim entity {}", (uint32_t)entry.SimEntity);
}