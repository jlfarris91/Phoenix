#pragma once

#include "RenderScene.h"
#include "Services/Service.h"

namespace Phoenix::Renderer
{
    class IRenderer : public Phoenix::IService
    {
        PHX_DECLARE_TYPE_DERIVED(IRenderer, Phoenix::IService)

    public:
        virtual void BeginFrame() = 0;
        virtual void Submit(const RenderScene& scene) = 0;
        virtual void EndFrame() = 0;
    };
}
