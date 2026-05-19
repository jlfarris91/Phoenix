#pragma once

#include <memory>
#include <SDL3/SDL.h>
#include "IRenderer.h"
#include "SDL3Context.h"
#include "SDL3ResourceManager.h"

namespace Phoenix::Renderer::SDL3
{
    class SDL3Renderer : public Phoenix::Renderer::IRenderer
    {
        PHX_DECLARE_TYPE_DERIVED(SDL3Renderer, Phoenix::Renderer::IRenderer)
    public:
        SDL3Renderer(std::shared_ptr<SDL3Context>         context,
                     std::shared_ptr<SDL3ResourceManager> resources);

        void BeginFrame() override;
        void Submit(const RenderScene& scene) override;
        void EndFrame() override;

    private:
        void DrawSprite(const SceneView& view, const Sprite2DCall& call);
        void DrawLine   (const SceneView& view, const Line2DCall&   call);
        void DrawCircle (const SceneView& view, const Circle2DCall& call);
        void DrawRect   (const SceneView& view, const Rect2DCall&   call);
        void DrawText   (const SceneView& view, const Text2DCall&   call);
        void DrawMesh   (const SceneView& view, const Mesh2DCall&   call);

        std::shared_ptr<SDL3Context>         Context;
        std::shared_ptr<SDL3ResourceManager> Resources;
    };
}
