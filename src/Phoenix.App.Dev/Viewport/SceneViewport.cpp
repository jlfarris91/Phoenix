#include "SceneViewport.h"

#include <entt/entt.hpp>

#include "Scene.h"
#include "SDL3Renderer.h"
#include "SDL3ResourceManager.h"
#include "Components/SceneComponent.h"
#include "Components/LineMesh2DComponent.h"
#include "RenderPrimitives.h"
#include "Components/Circle2DComponent.h"

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
        {
            Resources->ReleaseResource(RenderTarget);
            RenderTarget = {};
        }
    }

    void SceneViewport::Resize(int width, int height)
    {
        if (View.Width == width && View.Height == height)
            return;

        if (RenderTarget.IsValid())
        {
            Resources->ReleaseResource(RenderTarget);
            RenderTarget = {};
        }

        View.Width  = width;
        View.Height = height;

        if (width > 0 && height > 0)
        {
            RenderTarget = Resources->CreateRenderTarget(width, height);
        }
        else
        {
            RenderTarget = {};
        }
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

        auto circleView = reg.view<SceneComponent, Circle2DComponent>();
        for (auto entity : circleView)
        {
            const auto& sc  = circleView.get<SceneComponent>(entity);
            const auto& circleComp  = circleView.get<Circle2DComponent>(entity);

            auto pos3D = sc.WorldTransform * glm::vec4(circleComp.Center, 0, 1);
            auto pos2D = glm::vec2(pos3D.x, pos3D.y);

            Circle2DCall call;
            call.Center = pos2D;
            call.Radius = circleComp.Radius;
            call.Color  = circleComp.Color;
            call.Filled = circleComp.Filled;

            Scene.Commands.push_back({ sc.Layer, call });
        }

        auto lineMeshView = reg.view<SceneComponent, LineMesh2DComponent>();
        for (auto entity : lineMeshView)
        {
            const auto& sc  = lineMeshView.get<SceneComponent>(entity);
            const auto& meshComp  = lineMeshView.get<LineMesh2DComponent>(entity);

            auto mesh = meshComp.Mesh.LoadResource(*Resources);
            if (!mesh)
            {
                continue;
            }

            LineMesh2DCall call;
            call.Mesh = mesh->GetHandle();
            call.Tint = meshComp.Tint;
            call.Scale = glm::vec2(meshComp.Scale);
            call.Transform = sc.WorldTransform;

            Scene.Commands.push_back({ sc.Layer, call });
        }

        Renderer->Submit(Scene);
    }
}
