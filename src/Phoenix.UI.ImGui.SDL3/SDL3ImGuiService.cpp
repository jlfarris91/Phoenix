#include "SDL3ImGuiService.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#if defined(_WIN32)
#  include <Windows.h>
#  include <GL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include "SDL3PlatformService.h"
#include "Application/Application.h"

using namespace Phoenix;
using namespace Phoenix::UI;

void SDL3ImGuiService::Initialize(const std::shared_ptr<Application>& app)
{
    IImGuiService::Initialize(app);

    auto platform = app->ResolveService<SDL3PlatformService>();
    if (!platform)
        return;

    EventHandle      = platform->OnEvent.AddSP(
        shared_from_this(), &SDL3ImGuiService::HandleEvent);
    AfterPollHandle  = platform->OnAfterPollEvents.AddSP(
        shared_from_this(), &SDL3ImGuiService::BeginFrame);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForOpenGL(platform->GetWindow(), platform->GetGLContext());
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

void SDL3ImGuiService::Shutdown()
{
    if (auto app = GetApplication())
    {
        if (auto platform = app->GetService<SDL3PlatformService>())
        {
            platform->OnEvent.Remove(EventHandle);
            platform->OnAfterPollEvents.Remove(AfterPollHandle);
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    IImGuiService::Shutdown();
}

void SDL3ImGuiService::PostTick()
{
    ImGui::Render();

    auto app = GetApplication();
    if (!app) return;
    auto platform = app->GetService<SDL3PlatformService>();
    if (!platform) return;

    SDL_Window* window = platform->GetWindow();
    int w, h;
    SDL_GetWindowSizeInPixels(window, &w, &h);
    glViewport(0, 0, w, h);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        SDL_Window*   backupWindow  = SDL_GL_GetCurrentWindow();
        SDL_GLContext backupContext = SDL_GL_GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(backupWindow, backupContext);
    }

    SDL_GL_SwapWindow(window);
}

void SDL3ImGuiService::HandleEvent(SDL_Event& event)
{
    ImGui_ImplSDL3_ProcessEvent(&event);
}

void SDL3ImGuiService::BeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}
