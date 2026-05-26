#pragma once

#include <RenderScene.h>
#include "Phoenix.Sim/WorldsFwd.h"

namespace Phoenix::Scene
{
    class IScene
    {
        PHX_DECLARE_TYPE_INTERFACE(IScene)
    public:
        virtual ~IScene() = default;
        virtual void Sync(WorldConstRef world) = 0;
        virtual Renderer::RenderScene Gather(WorldConstRef world) const = 0;
    };
}
