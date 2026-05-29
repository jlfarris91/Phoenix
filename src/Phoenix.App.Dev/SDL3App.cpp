#include "SDL3App.h"

#include <imgui.h>

#include "SDL3PlatformService.h"
#include "SDL3ImGuiService.h"
#include "SDL3Renderer.h"
#include "SDL3ResourceManager.h"
#include "Viewport/ImGuiViewport.h"
#include "Viewport/SceneViewport.h"

namespace Phoenix::App::Dev
{
    SDL3App::SDL3App(std::shared_ptr<IServiceLocator> locator)
        : Application(std::move(locator))
    {
    }

    void SDL3App::InitializeInternal()
    {
        Application::InitializeInternal();

        Platform = ResolveService<SDL3PlatformService>();
        ImGuiSvc = ResolveService<SDL3ImGuiService>();
        Renderer = ResolveService<SDL3Renderer>();

        auto renderer  = ResolveService<SDL3Renderer>();
        auto resources = ResolveService<SDL3ResourceManager>();
        SceneVP = std::make_shared<SceneViewport>(renderer, resources);
        Viewport = std::make_shared<ImGuiViewport>(SceneVP, resources);
    }

    void SDL3App::Tick()
    {
        Platform->PollEvents();

        for (const auto& service : AppServices)
            service->PreTick();

        for (const auto& service : AppServices)
            service->Tick();

        for (const auto& service : AppServices)
            service->PostTick();

        SDL_RenderClear(Platform->GetRenderer());

        // Render Worlds???

        ImGuiSvc->BeginFrame();

        // Render UI
        Viewport->Draw();

        ImGuiSvc->EndFrame();

        SDL_RenderPresent(Platform->GetRenderer());
    }
}
