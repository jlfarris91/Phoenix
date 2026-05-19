#pragma once

#include <Phoenix.Sim/WorldsFwd.h>
#include <RenderScene.h>

namespace Phoenix
{
    class ISceneLayer
    {
    public:
        virtual ~ISceneLayer() = default;
        virtual void Gather(WorldConstRef world, Renderer::RenderScene& scene) = 0;
    };
}
