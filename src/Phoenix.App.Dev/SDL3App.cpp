#include "SDL3App.h"

#include <imgui.h>

#include "SDL3PlatformService.h"
#include "SDL3ImGuiService.h"
#include "SDL3Renderer.h"
#include "SDL3ResourceManager.h"
#include "Viewport/ImGuiViewport.h"
#include "Viewport/SceneViewport.h"

#include "SceneManager.h"
#include "Phoenix.Scene/IScene.h"
#include "Phoenix.App.Dev/Scene.h"

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

        // Just for a test, render the first scene
        for (auto&& scene : ResolveService<Phoenix::Scene::ISceneManager>()->GetAllScenes())
        {
            SceneVP->SetCenter({ 0.f, 0.f });
            SceneVP->SetPixelsPerUnit(10.0f);
            SceneVP->Render(*std::static_pointer_cast<Scene>(scene));
            break;
        }

        ImGuiSvc->BeginFrame();

        // Render UI
        Viewport->Draw();

        ImGuiSvc->EndFrame();

        SDL_RenderPresent(Platform->GetRenderer());
    }
}
