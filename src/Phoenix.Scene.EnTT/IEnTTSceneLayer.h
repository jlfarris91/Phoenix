#pragma once

#include <entt/entt.hpp>

#include <Phoenix.Sim/WorldsFwd.h>
#include <RenderScene.h>

namespace Phoenix
{
    class IEnTTSceneLayer
    {
    public:
        virtual ~IEnTTSceneLayer() = default;
        virtual void Gather(const entt::registry& registry, WorldConstRef world, Renderer::RenderScene& scene) = 0;
    };
}
