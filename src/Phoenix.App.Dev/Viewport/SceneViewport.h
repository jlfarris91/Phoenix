#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "RenderScene.h"
#include "SceneView.h"
#include "RendererTypes.h"

namespace Phoenix::App::Dev
{
    class Scene;
    class SDL3Renderer;
    class SDL3ResourceManager;

    // Owns a render target and knows how to gather and submit scene draw calls.
    class SceneViewport
    {
    public:
        using Dependencies = std::tuple<SDL3Renderer, SDL3ResourceManager>;
        SceneViewport(std::shared_ptr<SDL3Renderer> renderer, std::shared_ptr<SDL3ResourceManager> resources);
        ~SceneViewport();

        // Resize the render target. Call when the display region changes.
        void Resize(int width, int height);

        // Camera configuration.
        void SetCenter(glm::vec2 center);
        void SetPixelsPerUnit(float ppu);

        glm::vec2       GetCenter()        const { return View.Center; }
        float           GetPixelsPerUnit() const { return View.PixelsPerUnit; }
        int             GetWidth()         const { return View.Width; }
        int             GetHeight()        const { return View.Height; }
        HRenderTarget   GetRenderTarget() const { return RenderTarget; }

        // Gather draw calls from the scene registry and submit to the renderer.
        void Render(const Scene& scene);

    private:
        std::shared_ptr<SDL3Renderer>        Renderer;
        std::shared_ptr<SDL3ResourceManager> Resources;

        SceneView     View;
        HRenderTarget RenderTarget;
        RenderScene   Scene;
    };
}
