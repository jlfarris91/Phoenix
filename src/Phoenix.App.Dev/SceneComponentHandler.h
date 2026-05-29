#pragma once

#include <entt/entt.hpp>

#include "Phoenix/Reflection/Registration.h"
#include "Phoenix.Sim/WorldsFwd.h"
#include "Phoenix.Sim.ECS/EntityId.h"

namespace Phoenix::App::Dev
{
    class Scene;

    struct SceneComponentSyncArgs
    {
        // The world containing the sim entity.
        WorldConstPtr World = nullptr;

        // The scene containing the scene entity.
        Scene* Scene = nullptr;

        // The entity in the scene.
        entt::entity SceneEntity;

        // The entity in the sim.
        ECS::EntityId SimEntity;

        // The type id of the component.
        FName SimComponentTypeId;

        // The raw data of the component.
        const void* SimComponentData = nullptr;
    };

    class ISceneComponentHandler
    {
        PHX_DECLARE_TYPE_INTERFACE(ISceneComponentHandler)
    public:
        virtual ~ISceneComponentHandler() = default;

        virtual void Register(const std::weak_ptr<Scene>& scene) {};

        virtual void Unregister() {};

        // Return true if the handler can handle the component for a given entity.
        virtual bool CanSync(const SceneComponentSyncArgs& args) = 0;

        //
        virtual void OnSync(const SceneComponentSyncArgs& args) = 0;
    };
}
