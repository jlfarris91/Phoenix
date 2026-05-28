#pragma once

#include "RendererTypes.h"
#include "SceneProxy.h"
#include "Transform2D.h"

namespace Phoenix::Renderer
{
    class LineMesh2DProxy : public ISceneProxy
    {
        PHX_DECLARE_TYPE_DERIVED(LineMesh2DProxy, ISceneProxy)
    public:

        void Gather(RenderScene& scene) override;

        int32_t     Layer = 0;
        HLineMesh2D Mesh;
        HTexture    Texture;
        glm::mat3   Transform;
        Color4b     Tint = Color4b::White();
    };
}
