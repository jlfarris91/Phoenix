#include "SceneViewport.h"

#include <entt/entt.hpp>

#include "Scene.h"
#include "SDL3Renderer.h"
#include "SDL3ResourceManager.h"
#include "Components/SceneComponent.h"
#include "Components/LineMesh2DComponent.h"
#include "RenderPrimitives.h"

namespace Phoenix::App::Dev
{
    SceneViewport::SceneViewport(std::shared_ptr<SDL3Renderer>       renderer,
                                  std::shared_ptr<SDL3ResourceManager> resources)
        : Renderer(std::move(renderer))
        , Resources(std::move(resources))
    {
    }

    SceneViewport::~SceneViewport()
    {
        if (RenderTarget.IsValid())
            Resources->ReleaseRenderTarget(RenderTarget);
    }

    void SceneViewport::Resize(int width, int height)
    {
        if (View.Width == width && View.Height == height)
            return;

        if (RenderTarget.IsValid())
            Resources->ReleaseRenderTarget(RenderTarget);

        View.Width  = width;
        View.Height = height;

        if (width > 0 && height > 0)
            RenderTarget = Resources->CreateRenderTarget(width, height);
        else
            RenderTarget = {};
    }

    void SceneViewport::SetCenter(glm::vec2 center)
    {
        View.Center = center;
    }

    void SceneViewport::SetPixelsPerUnit(float ppu)
    {
        View.PixelsPerUnit = ppu;
    }

    void SceneViewport::Render(const Dev::Scene& scene)
    {
        if (!RenderTarget.IsValid())
            return;

        Scene.View   = View;
        Scene.Target = RenderTarget;
        Scene.Clear();

        const entt::registry& reg = scene.GetRegistry();

        auto lineMeshView = reg.view<SceneComponent, LineMesh2DComponent>();
        for (auto entity : lineMeshView)
        {
            const auto& sc  = lineMeshView.get<SceneComponent>(entity);
            const auto& lm  = lineMeshView.get<LineMesh2DComponent>(entity);

            if (!lm.Mesh.IsValid())
                continue;

            LineMesh2DCall call;
            call.Mesh      = lm.Mesh;
            call.Texture   = lm.Texture;
            call.Transform = sc.LocalTransform;
            call.Tint      = lm.Tint;

            Scene.Commands.push_back({ sc.Layer, call });
        }

        Renderer->Submit(Scene);
    }
}
