#pragma once

#include "Application/Application.h"
#include "Phoenix/Reflection/Registration.h"

namespace Phoenix::App::Dev
{
    class SDL3Renderer;
    class SDL3PlatformService;
    class SDL3ImGuiService;
    class SceneViewport;
    class ImGuiViewport;

    class SDL3App : public Application
    {
        PHX_DECLARE_TYPE_DERIVED(SDL3App, Application)
    public:
        explicit SDL3App(std::shared_ptr<IServiceLocator> locator);

        void Tick() override;

    protected:

        void InitializeInternal() override;

    private:
        std::shared_ptr<SDL3PlatformService>    Platform;
        std::shared_ptr<SDL3Renderer>           Renderer;
        std::shared_ptr<SDL3ImGuiService>       ImGuiSvc;
        std::shared_ptr<SceneViewport>          SceneVP;
        std::shared_ptr<ImGuiViewport>          Viewport;
    };
}
