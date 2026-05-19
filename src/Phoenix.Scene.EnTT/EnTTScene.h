#pragma once

#include <memory>
#include <vector>

#include <entt/entt.hpp>

#include <Phoenix.Sim/WorldsFwd.h>
#include <RenderScene.h>
#include <SceneCamera.h>
#include <RendererTypes.h>
#include <SceneEntitySync.h>

#include "IEnTTSceneLayer.h"

namespace Phoenix
{
    class EnTTScene
    {
    public:
        SceneCamera                                   Camera;
        Renderer::HRenderTarget                       Target;
        std::vector<std::shared_ptr<IEnTTSceneLayer>> Layers;
        SceneEntitySync                               EntitySync;

        entt::registry&       GetRegistry()       { return Registry; }
        const entt::registry& GetRegistry() const { return Registry; }

        Renderer::RenderScene Gather(WorldConstRef world);

    private:
        entt::registry Registry;
    };
}
