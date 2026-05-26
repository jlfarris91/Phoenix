#pragma once

#include "Scene.h"
#include "SceneComponentHandler.h"

namespace Phoenix::EnTT
{
    template <class TSimComponent, class TSceneComponent>
    class AutoSceneComponentHandler : public ISceneComponentHandler
    {
        PHX_DECLARE_TYPE_DERIVED(AutoSceneComponentHandler, ISceneComponentHandler)
    public:

        bool CanHandleSimComponent(const SceneComponentHandlerArgs& args) override
        {
            return args.SimComponentTypeId == StaticTypeName<TSimComponent>::TypeId;
        }

        void OnSpawnComponent(const SceneComponentHandlerArgs& args) override
        {
            if constexpr (requires (TSceneComponent& c, const SceneComponentHandlerArgs& a) { c.OnSpawn(a); })
            {
                auto& sceneComp = args.Scene->GetRegistry().get_or_emplace<TSceneComponent>(args.SceneEntity);
                sceneComp.OnSpawn(args);
            }
        }

        void OnUpdateComponent(const SceneComponentHandlerArgs& args) override
        {
            if constexpr (requires (TSceneComponent& c, const SceneComponentHandlerArgs& a) { c.OnUpdate(a); })
            {
                auto& sceneComp = args.Scene->GetRegistry().get_or_emplace<TSceneComponent>(args.SceneEntity);
                sceneComp.OnUpdate(args);
            }
        }

        void OnDestroyComponent(const SceneComponentHandlerArgs& args) override
        {
            if constexpr (requires (TSceneComponent& c, const SceneComponentHandlerArgs& a) { c.OnDestroy(a); })
            {
                auto& sceneComp = args.Scene->GetRegistry().get_or_emplace<TSceneComponent>(args.SceneEntity);
                sceneComp.OnDestroy(args);
            }
        }
    };
}

#define PHX_DECLARE_ENTT_AUTO_COMP(simComp, sceneComp) \
    public: \
        using AutoSceneComponentHandlerT = class Phoenix::EnTT::AutoSceneComponentHandler<simComp, sceneComp>; \
        template <simComp, sceneComp> friend class Phoenix::EnTT::AutoSceneComponentHandler;