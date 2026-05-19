#pragma once

#include <memory>
#include <vector>

#include <RenderScene.h>
#include "SceneCamera.h"
#include "ISceneLayer.h"

namespace Phoenix
{
    class Scene
    {
    public:
        SceneCamera                              Camera;
        Renderer::HRenderTarget                  Target;
        std::vector<std::shared_ptr<ISceneLayer>> Layers;

        Renderer::RenderScene Gather(WorldConstRef world) const;
    };
}
