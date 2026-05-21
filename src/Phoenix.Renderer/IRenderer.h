#pragma once

#include "RenderScene.h"
#include "Phoenix/Services/IService.h"

namespace Phoenix::Renderer
{
    class IRenderer : public IService
    {
        PHX_DECLARE_TYPE_DERIVED(IRenderer, Phoenix::IService)

    public:
        virtual void BeginFrame() = 0;
        virtual void Submit(const RenderScene& scene) = 0;
        virtual void EndFrame() = 0;
    };
}
