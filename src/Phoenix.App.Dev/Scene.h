#pragma once

#include <entt/entt.hpp>

#include <Phoenix.Sim/WorldsFwd.h>
#include <WorldSceneSync.h>

#include "IScene.h"

namespace Phoenix::App::Dev
{
    class ISceneComponentHandler;

    class Scene : public Phoenix::Scene::IScene,
                  public std::enable_shared_from_this<Scene>
    {
        PHX_DECLARE_TYPE_DERIVED(Scene, Phoenix::Scene::IScene)
    public:

        Scene(FName id);

        entt::registry&       GetRegistry();
        const entt::registry& GetRegistry() const;

        void Sync(WorldConstRef world) override;

        void RegisterSyncComponentHandler(const std::shared_ptr<ISceneComponentHandler>& handler);
        bool UnregisterSyncComponentHandler(const std::shared_ptr<ISceneComponentHandler>& handler);

        template <class T, class ...TArgs>
        void RegisterSyncComponentHandler(TArgs&& ...args)
        {
            RegisterSyncComponentHandler(std::make_shared<T>(std::forward<TArgs>(args)...));
        }

    private:

        Phoenix::Scene::SceneEntity OnSpawnEntity(
            WorldConstRef world,
            const Phoenix::Scene::WorldSceneSync::SyncEntry& entry);

        void OnUpdateEntity(
            WorldConstRef world,
            const Phoenix::Scene::WorldSceneSync::SyncEntry& entry);

        void OnDestroyEntity(
            WorldConstRef world,
            const Phoenix::Scene::WorldSceneSync::SyncEntry& entry);

        FName Id;
        Phoenix::Scene::WorldSceneSync EntitySync;
        std::vector<std::shared_ptr<ISceneComponentHandler>> SceneComponentHandlers;
        entt::registry Registry;
    };
}
