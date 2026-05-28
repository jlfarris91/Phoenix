#pragma once

#include "Phoenix/Services/IService.h"
#include "Phoenix.App/Dispatch.h"

namespace Phoenix::Renderer
{
    struct RenderScene;

    class IRenderer : public IService, public IDispatcher
    {
        PHX_DECLARE_TYPE_DERIVED(IRenderer, Phoenix::IService, Phoenix::IDispatcher)
    public:
        virtual void BeginFrame() = 0;
        virtual void Submit(const RenderScene& scene) = 0;
        virtual void EndFrame() = 0;
    };
}
