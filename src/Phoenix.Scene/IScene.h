#pragma once

#include "Phoenix.Sim/WorldsFwd.h"

namespace Phoenix::Renderer
{
    class ISceneProxyManager;
}

namespace Phoenix::Scene
{
    class IScene
    {
        PHX_DECLARE_TYPE_INTERFACE(IScene)
    public:
        virtual ~IScene() = default;
        virtual void Sync(WorldConstRef world) = 0;
        virtual Renderer::ISceneProxyManager& GetProxyManager() const = 0;
    };
}
