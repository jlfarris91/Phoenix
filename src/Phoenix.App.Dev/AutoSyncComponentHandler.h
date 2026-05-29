#pragma once

#include "Scene.h"
#include "SceneComponentHandler.h"

namespace Phoenix::App::Dev
{
    template <class TSimComponent, class TSceneComponent>
    class AutoSceneComponentHandler : public ISceneComponentHandler
    {
        PHX_DECLARE_TYPE_DERIVED(AutoSceneComponentHandler, ISceneComponentHandler)
    public:
        ~AutoSceneComponentHandler() override
        {
            AutoSceneComponentHandler::Unregister();
        }

        void Register(const std::weak_ptr<Scene>& weakScenePtr) override
        {
            WeakScenePtr = weakScenePtr;
            if (auto scenePtr = weakScenePtr.lock())
            {
                auto& registry = scenePtr->GetRegistry();
                registry.on_construct<TSceneComponent>().connect<&AutoSceneComponentHandler::OnConstruct>(this);
                registry.on_update<TSceneComponent>().connect<&AutoSceneComponentHandler::OnUpdate>(this);
                registry.on_destroy<TSceneComponent>().connect<&AutoSceneComponentHandler::OnDestroy>(this);
            }
        }

        void Unregister() override
        {
            if (auto scenePtr = WeakScenePtr.lock())
            {
                auto& registry = scenePtr->GetRegistry();
                registry.on_construct<TSceneComponent>().disconnect<&AutoSceneComponentHandler::OnConstruct>(this);
                registry.on_update<TSceneComponent>().disconnect<&AutoSceneComponentHandler::OnUpdate>(this);
                registry.on_destroy<TSceneComponent>().disconnect<&AutoSceneComponentHandler::OnDestroy>(this);
            }
            WeakScenePtr.reset();
        }

        void OnConstruct(entt::registry& registry, entt::entity entity)
        {
            if constexpr (requires (TSceneComponent& c, Scene& s, entt::entity e) { c.OnConstruct(s, e); })
            {
                if (auto scenePtr = WeakScenePtr.lock())
                {
                    registry.get<TSceneComponent>(entity).OnConstruct(*scenePtr, entity);
                }
            }
        }

        void OnUpdate(entt::registry& registry, entt::entity entity)
        {
            if constexpr (requires (TSceneComponent& c, Scene& s, entt::entity e) { c.OnUpdate(s, e); })
            {
                if (auto scenePtr = WeakScenePtr.lock())
                {
                    registry.get<TSceneComponent>(entity).OnUpdate(*scenePtr, entity);
                }
            }
        }

        void OnDestroy(entt::registry& registry, entt::entity entity)
        {
            if constexpr (requires (TSceneComponent& c, Scene& s, entt::entity e) { c.OnDestroy(s, e); })
            {
                if (auto scenePtr = WeakScenePtr.lock())
                {
                    registry.get<TSceneComponent>(entity).OnDestroy(*scenePtr, entity);
                }
            }
        }

        bool CanSync(const SceneComponentSyncArgs& args) override
        {
            PHX_ASSERT(args.Scene == WeakScenePtr.lock().get());
            return args.SimComponentTypeId == StaticTypeName<TSimComponent>::TypeId;
        }

        void OnSync(const SceneComponentSyncArgs& args) override
        {
            PHX_ASSERT(args.Scene == WeakScenePtr.lock().get());
            if constexpr (requires (TSceneComponent& c, const SceneComponentSyncArgs& a) { c.OnSync(a); })
            {
                auto& sceneComp = args.Scene->GetRegistry().get_or_emplace<TSceneComponent>(args.SceneEntity);
                sceneComp.OnSync(args);
            }
        }

    private:
        std::weak_ptr<Scene> WeakScenePtr;
    };
}

#define PHX_DECLARE_ENTT_AUTO_COMP(simComp, sceneComp) \
    public: \
        using AutoSceneComponentHandlerT = class Phoenix::App::Dev::AutoSceneComponentHandler<simComp, sceneComp>; \
        template <simComp, sceneComp> friend class Phoenix::App::Dev::AutoSceneComponentHandler;