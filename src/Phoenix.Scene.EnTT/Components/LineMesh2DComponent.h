#pragma once

#include <entt/entt.hpp>

#include "RendererTypes.h"

namespace Phoenix::EnTT
{
    class Scene;

    struct LineMesh2DComponent
    {
        void OnConstruct(Scene& scene, entt::entity entity);
        void OnUpdate(Scene& scene, entt::entity entity);
        void OnDestroy(Scene& scene, entt::entity entity);

        int32_t                     Layer = 0;
        Renderer::HLineMesh2D       Mesh;
        Renderer::HTexture          Texture;
        Color4b                     Tint = Color4b::White();
        Renderer::SceneProxyHandle  ProxyHandle;
    };
}
