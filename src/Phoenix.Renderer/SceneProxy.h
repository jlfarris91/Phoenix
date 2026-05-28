#pragma once

#include "Phoenix/Reflection/Registration.h"

namespace Phoenix::Renderer
{
    struct RenderScene;

    class ISceneProxy
    {
        PHX_DECLARE_TYPE_INTERFACE(ISceneProxy)
    public:
        virtual ~ISceneProxy() {}
        virtual void Gather(RenderScene& scene) = 0;
    };
}
