#pragma once

#include <memory>
#include "Application/AppService.h"
#include "RenderScene.h"
#include "SDL3PlatformService.h"
#include "SDL3ResourceManager.h"

#include "Phoenix/Reflection/Registration.h"

namespace Phoenix::App::Dev
{
    class SDL3Renderer : public IAppService
    {
        PHX_DECLARE_TYPE_DERIVED(SDL3Renderer, IAppService)
    public:
        using Dependencies = std::tuple<SDL3PlatformService, SDL3ResourceManager>;
        SDL3Renderer(std::shared_ptr<SDL3PlatformService> platform, std::shared_ptr<SDL3ResourceManager> resources);

        void Submit(const RenderScene& scene);
        void EndFrame();

    private:
        void DrawSprite    (const SceneView& view, const Sprite2DCall&    call);
        void DrawLine      (const SceneView& view, const Line2DCall&      call);
        void DrawCircle    (const SceneView& view, const Circle2DCall&    call);
        void DrawRect      (const SceneView& view, const Rect2DCall&      call);
        void DrawText      (const SceneView& view, const Text2DCall&      call);
        void DrawMesh      (const SceneView& view, const Mesh2DCall&      call);
        void DrawLineMesh  (const SceneView& view, const LineMesh2DCall&  call);

        std::shared_ptr<SDL3PlatformService> Platform;
        std::shared_ptr<SDL3ResourceManager> Resources;
    };
}
