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
            SceneVP->SetPixelsPerUnit(50.0f);
            SceneVP->Render(*std::static_pointer_cast<Scene>(scene));
            break;
        }

        ImGuiSvc->BeginFrame();

        // Render UI
        {
            const ::ImGuiViewport* vp = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(vp->WorkPos);
            ImGui::SetNextWindowSize(vp->WorkSize);
            ImGui::SetNextWindowViewport(vp->ID);

            ImGuiWindowFlags hostFlags =
                ImGuiWindowFlags_NoDocking             |
                ImGuiWindowFlags_NoTitleBar            |
                ImGuiWindowFlags_NoCollapse            |
                ImGuiWindowFlags_NoResize              |
                ImGuiWindowFlags_NoMove                |
                ImGuiWindowFlags_NoBringToFrontOnFocus |
                ImGuiWindowFlags_NoNavFocus            ;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,   0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    ImVec2(0.0f, 0.0f));
            ImGui::Begin("##RootDockSpace", nullptr, hostFlags);
            ImGui::PopStyleVar(3);

            Viewport->Draw();

            ImGui::End();
        }

        ImGuiSvc->EndFrame();

        SDL_RenderPresent(Platform->GetRenderer());
    }
}
