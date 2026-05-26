#pragma once

#include <entt/entity/entity.hpp>

#include "Phoenix/Reflection/Registration.h"
#include "Phoenix.Sim.ECS/EntityId.h"

namespace Phoenix::EnTT
{
    class Scene;

    struct SceneComponentHandlerArgs
    {
        WorldConstPtr World = nullptr;
        Scene* Scene = nullptr;
        entt::entity SceneEntity;
        ECS::EntityId SimEntity;
        FName SimComponentTypeId;
        const void* SimComponentData = nullptr;
    };

    class ISceneComponentHandler
    {
        PHX_DECLARE_TYPE_INTERFACE(ISceneComponentHandler)
    public:
        virtual ~ISceneComponentHandler() = default;

        // Return true if the handler can handle the component for a given entity.
        virtual bool CanHandleSimComponent(const SceneComponentHandlerArgs& args) = 0;

        //
        virtual void OnSpawnComponent(const SceneComponentHandlerArgs& args) = 0;

        //
        virtual void OnUpdateComponent(const SceneComponentHandlerArgs& args) = 0;

        //
        virtual void OnDestroyComponent(const SceneComponentHandlerArgs& args) = 0;
    };
}
