#include "SDL3ImGuiService.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include "SDL3PlatformService.h"
#include "Application/Application.h"

using namespace Phoenix;
using namespace Phoenix::UI;
using namespace Phoenix::App::Dev;

void SDL3ImGuiService::Initialize(const std::shared_ptr<Application>& app)
{
    IImGuiService::Initialize(app);

    Platform = app->ResolveService<SDL3PlatformService>();
    Renderer = Platform->GetRenderer();

    auto self = std::static_pointer_cast<SDL3ImGuiService>(shared_from_this());
    EventHandle         = Platform->OnEvent.AddSP(self, &SDL3ImGuiService::HandleEvent);
    DisplayScaleHandle  = Platform->OnDisplayScaleChanged.AddSP(self, &SDL3ImGuiService::OnDisplayScaleChanged);

    float dpiScale = Platform->GetDisplayScale();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    SetupStyle(dpiScale);

    ImGui_ImplSDL3_InitForSDLRenderer(Platform->GetWindow(), Renderer);
    ImGui_ImplSDLRenderer3_Init(Renderer);
}

void SDL3ImGuiService::Shutdown()
{
    if (auto app = GetApplication())
    {
        if (auto platform = app->GetService<SDL3PlatformService>())
        {
            platform->OnEvent.Remove(EventHandle);
            platform->OnDisplayScaleChanged.Remove(DisplayScaleHandle);
        }
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    IImGuiService::Shutdown();
}

void SDL3ImGuiService::BeginFrame()
{
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void SDL3ImGuiService::EndFrame()
{
    ImGui::Render();

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    SDL_SetRenderDrawColorFloat(Renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), Renderer);
}

void SDL3ImGuiService::HandleEvent(SDL_Event& event)
{
    ImGui_ImplSDL3_ProcessEvent(&event);
}

void SDL3ImGuiService::OnDisplayScaleChanged(float scale)
{
    ImGui::GetStyle() = ImGuiStyle{};
    SetupStyle(scale);
}

void SDL3ImGuiService::SetupStyle(float dpiScale)
{
    ImGuiStyle& style = ImGui::GetStyle();

    style.Alpha                     = 1.0f;
    style.DisabledAlpha             = 0.6f;
    style.WindowPadding             = ImVec2(8.0f, 8.0f);
    style.WindowRounding            = 8.4f;
    style.WindowBorderSize          = 1.0f;
    style.WindowMinSize             = ImVec2(32.0f, 32.0f);
    style.WindowTitleAlign          = ImVec2(0.0f, 0.5f);
    style.WindowMenuButtonPosition  = ImGuiDir_Right;
    style.ChildRounding             = 3.0f;
    style.ChildBorderSize           = 1.0f;
    style.PopupRounding             = 3.0f;
    style.PopupBorderSize           = 1.0f;
    style.FramePadding              = ImVec2(4.0f, 3.0f);
    style.FrameRounding             = 3.0f;
    style.FrameBorderSize           = 1.0f;
    style.ItemSpacing               = ImVec2(8.0f, 4.0f);
    style.ItemInnerSpacing          = ImVec2(4.0f, 4.0f);
    style.CellPadding               = ImVec2(4.0f, 2.0f);
    style.IndentSpacing             = 21.0f;
    style.ColumnsMinSpacing         = 6.0f;
    style.ScrollbarSize             = 5.6f;
    style.ScrollbarRounding         = 18.0f;
    style.GrabMinSize               = 10.0f;
    style.GrabRounding              = 3.0f;
    style.TabRounding               = 3.0f;
    style.TabBorderSize             = 0.0f;
    style.ColorButtonPosition       = ImGuiDir_Right;
    style.ButtonTextAlign           = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign       = ImVec2(0.0f, 0.0f);

    style.Colors[ImGuiCol_Text]                  = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.125f, 0.125f, 0.125f, 1.00f);
    style.Colors[ImGuiCol_ChildBg]               = ImVec4(0.125f, 0.125f, 0.125f, 1.00f);
    style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.169f, 0.169f, 0.169f, 1.00f);
    style.Colors[ImGuiCol_Border]                = ImVec4(0.251f, 0.251f, 0.251f, 1.00f);
    style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f,  0.00f,  0.00f,  0.00f);
    style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.169f, 0.169f, 0.169f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.216f, 0.216f, 0.216f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.251f, 0.251f, 0.251f, 1.00f);
    style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.125f, 0.125f, 0.125f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.169f, 0.169f, 0.169f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.125f, 0.125f, 0.125f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.169f, 0.169f, 0.169f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.125f, 0.125f, 0.125f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.251f, 0.251f, 0.251f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.302f, 0.302f, 0.302f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.349f, 0.349f, 0.349f, 1.00f);
    style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.00f,  0.471f, 0.843f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.00f,  0.471f, 0.843f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.00f,  0.329f, 0.600f, 1.00f);
    style.Colors[ImGuiCol_Button]                = ImVec4(0.169f, 0.169f, 0.169f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.216f, 0.216f, 0.216f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.251f, 0.251f, 0.251f, 1.00f);
    style.Colors[ImGuiCol_Header]                = ImVec4(0.216f, 0.216f, 0.216f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.251f, 0.251f, 0.251f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.302f, 0.302f, 0.302f, 1.00f);
    style.Colors[ImGuiCol_Separator]             = ImVec4(0.216f, 0.216f, 0.216f, 1.00f);
    style.Colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.251f, 0.251f, 0.251f, 1.00f);
    style.Colors[ImGuiCol_SeparatorActive]       = ImVec4(0.302f, 0.302f, 0.302f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.216f, 0.216f, 0.216f, 1.00f);
    style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.251f, 0.251f, 0.251f, 1.00f);
    style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.302f, 0.302f, 0.302f, 1.00f);
    style.Colors[ImGuiCol_Tab]                   = ImVec4(0.169f, 0.169f, 0.169f, 1.00f);
    style.Colors[ImGuiCol_TabHovered]            = ImVec4(0.216f, 0.216f, 0.216f, 1.00f);
    style.Colors[ImGuiCol_TabActive]             = ImVec4(0.251f, 0.251f, 0.251f, 1.00f);
    style.Colors[ImGuiCol_TabUnfocused]          = ImVec4(0.169f, 0.169f, 0.169f, 1.00f);
    style.Colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.216f, 0.216f, 0.216f, 1.00f);
    style.Colors[ImGuiCol_PlotLines]             = ImVec4(0.00f,  0.471f, 0.843f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.00f,  0.329f, 0.600f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.00f,  0.471f, 0.843f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(0.00f,  0.329f, 0.600f, 1.00f);
    style.Colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.188f, 0.188f, 0.200f, 1.00f);
    style.Colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.310f, 0.310f, 0.349f, 1.00f);
    style.Colors[ImGuiCol_TableBorderLight]      = ImVec4(0.227f, 0.227f, 0.247f, 1.00f);
    style.Colors[ImGuiCol_TableRowBg]            = ImVec4(0.00f,  0.00f,  0.00f,  0.00f);
    style.Colors[ImGuiCol_TableRowBgAlt]         = ImVec4(1.00f,  1.00f,  1.00f,  0.06f);
    style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.00f,  0.471f, 0.843f, 1.00f);
    style.Colors[ImGuiCol_DragDropTarget]        = ImVec4(1.00f,  1.00f,  0.00f,  0.90f);
    style.Colors[ImGuiCol_NavHighlight]          = ImVec4(0.259f, 0.588f, 0.976f, 1.00f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f,  1.00f,  1.00f,  0.70f);
    style.Colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f,  0.80f,  0.80f,  0.20f);
    style.Colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.80f,  0.80f,  0.80f,  0.35f);

    style.ScaleAllSizes(dpiScale);
}
