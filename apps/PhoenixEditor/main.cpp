#include <fstream>
#include <SDL3/SDL.h>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include <nlohmann/json.hpp>

#include "Widgets/EditorWidgetContext.h"
#include "Widgets/MainFrameWidget.h"
#include "Docking/ImGuiDockManager.h"
#include "Docking/ImGuiDockRenderer.h"
#include "Docking/ImGuiDockTab.h"
#include "Application/Application.h"
#include "Services/ServiceContainerBuilder.h"
#include "UI/Menu/MenuManager.h"

#if defined(__APPLE__)
#  include <OpenGL/gl.h>
#elif defined(_WIN32)
#  include <Windows.h>
#  include <GL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include <filesystem>
#include <string>

#include <Style/EditorStyle.h>
#include <Style/EditorFonts.h>

#include "Editor/Editor.h"

std::shared_ptr<Phoenix::Editor> GEditor;
std::shared_ptr<Phoenix::UI::MainFrameWidget> GMainFrameWidget;

// ---------------------------------------------------------------------------
// Forward declarations
// ---------------------------------------------------------------------------
static bool InitSDL(SDL_Window*& outWindow, SDL_GLContext& outContext);
static void ShutdownSDL(SDL_Window* window, SDL_GLContext context);
static void RenderFrame(SDL_Window* window, bool& outRunning);
static void RenderOneFrame(SDL_Window* window, bool& running, ImGuiIO& io);
static void SetupImGuiStyle();

static void InitializeApplication();

// Renders a frame from within the SDL event watch so the window stays live
// during the Win32 modal resize/move loop, which blocks SDL_PollEvent.
struct WindowEventWatchData { SDL_Window* window; bool* running; ImGuiIO* io; };
static bool OnWindowSizeOrMove(void* userdata, SDL_Event* event)
{
    if (event->type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED ||
        event->type == SDL_EVENT_WINDOW_MOVED)
    {
        auto* d = static_cast<WindowEventWatchData*>(userdata);
        RenderOneFrame(d->window, *d->running, *d->io);
    }
    return true;
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------
int main(int /*argc*/, char* /*argv*/[])
{
    SDL_Window*   window    = nullptr;
    SDL_GLContext glContext = nullptr;

    if (!InitSDL(window, glContext))
        return 1;

    // Resolve paths relative to the executable so the editor works regardless
    // of the working directory.
    const char* basePath = SDL_GetBasePath();
    std::filesystem::path exeDir(basePath ? basePath : ".");
    std::filesystem::path iniPath     = exeDir / "imgui.ini";
    std::filesystem::path defaultPath = exeDir / "default_layout.ini";

    if (!std::filesystem::exists(iniPath) && std::filesystem::exists(defaultPath))
        std::filesystem::copy_file(defaultPath, iniPath);

    // Store ini path as a persistent string — ImGui only holds a pointer.
    std::string iniPathStr = iniPath.string();

    float dpiScale = SDL_GetWindowDisplayScale(window);
    if (dpiScale <= 0.0f) dpiScale = 1.0f;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = iniPathStr.c_str();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    Phoenix::UI::Shell::GFonts = Phoenix::UI::Shell::LoadEditorFonts(exeDir / "data", 16.0f * dpiScale);

    // Phoenix::UI::Shell::ApplyEditorStyle();
    SetupImGuiStyle();
    ImGui::GetStyle().ScaleAllSizes(dpiScale);

    ImGui_ImplSDL3_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    InitializeApplication();

    bool running = true;
    WindowEventWatchData watchData{ window, &running, &io };
    SDL_AddEventWatch(OnWindowSizeOrMove, &watchData);

    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                running = false;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
                event.window.windowID == SDL_GetWindowID(window))
                running = false;
            if (event.type == SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED &&
                event.window.windowID == SDL_GetWindowID(window))
            {
                dpiScale = SDL_GetWindowDisplayScale(window);
                if (dpiScale <= 0.0f) dpiScale = 1.0f;
                io.Fonts->Clear();
                Phoenix::UI::Shell::GFonts = Phoenix::UI::Shell::LoadEditorFonts(exeDir / "data", 16.0f * dpiScale);
                ImGui::GetStyle() = ImGuiStyle{};
                SetupImGuiStyle();
                ImGui::GetStyle().ScaleAllSizes(dpiScale);
            }
        }

        RenderOneFrame(window, running, io);
    }

    SDL_RemoveEventWatch(OnWindowSizeOrMove, &watchData);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    ShutdownSDL(window, glContext);

    return 0;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static bool InitSDL(SDL_Window*& outWindow, SDL_GLContext& outContext)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,         0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,  SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,          1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,            24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,          8);

    SDL_WindowFlags flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    outWindow = SDL_CreateWindow("Phoenix", 1600, 900, flags);
    if (!outWindow)
    {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return false;
    }

    outContext = SDL_GL_CreateContext(outWindow);
    if (!outContext)
    {
        SDL_Log("SDL_GL_CreateContext failed: %s", SDL_GetError());
        SDL_DestroyWindow(outWindow);
        SDL_Quit();
        return false;
    }

    SDL_GL_MakeCurrent(outWindow, outContext);
    SDL_GL_SetSwapInterval(1); // vsync
    return true;
}

static void ShutdownSDL(SDL_Window* window, SDL_GLContext context)
{
    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

static void RenderFrame(SDL_Window* /*window*/, bool& outRunning)
{
    GMainFrameWidget->Render();
}

static void RenderOneFrame(SDL_Window* window, bool& running, ImGuiIO& io)
{
    // UpdatePlatformWindows moves SDL windows, which fires SDL_EVENT_WINDOW_MOVED and
    // re-enters this function via the event watch. Guard against that reentrancy.
    static bool sRendering = false;
    if (sRendering) return;
    sRendering = true;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    RenderFrame(window, running);

    ImGui::Render();

    int w, h;
    SDL_GetWindowSizeInPixels(window, &w, &h);
    glViewport(0, 0, w, h);
    glClearColor(0.08f, 0.08f, 0.09f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
        SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }

    SDL_GL_SwapWindow(window);
    sRendering = false;
}

void SetupImGuiStyle()
{
    // Windark style by DestroyerDarkNess from ImThemes
    ImGuiStyle& style = ImGui::GetStyle();
    
    style.Alpha = 1.0f;
    style.DisabledAlpha = 0.6f;
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.WindowRounding = 8.4f;
    style.WindowBorderSize = 1.0f;
    style.WindowMinSize = ImVec2(32.0f, 32.0f);
    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Right;
    style.ChildRounding = 3.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupRounding = 3.0f;
    style.PopupBorderSize = 1.0f;
    style.FramePadding = ImVec2(4.0f, 3.0f);
    style.FrameRounding = 3.0f;
    style.FrameBorderSize = 1.0f;
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
    style.CellPadding = ImVec2(4.0f, 2.0f);
    style.IndentSpacing = 21.0f;
    style.ColumnsMinSpacing = 6.0f;
    style.ScrollbarSize = 5.6f;
    style.ScrollbarRounding = 18.0f;
    style.GrabMinSize = 10.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 3.0f;
    style.TabBorderSize = 0.0f;
    // style.TabMinWidthForCloseButton = 0.0f;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);
    
    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1254902f, 0.1254902f, 0.1254902f, 1.0f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.1254902f, 0.1254902f, 0.1254902f, 1.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.16862746f, 0.16862746f, 0.16862746f, 1.0f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.2509804f, 0.2509804f, 0.2509804f, 1.0f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.16862746f, 0.16862746f, 0.16862746f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.21568628f, 0.21568628f, 0.21568628f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2509804f, 0.2509804f, 0.2509804f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1254902f, 0.1254902f, 0.1254902f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.16862746f, 0.16862746f, 0.16862746f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1254902f, 0.1254902f, 0.1254902f, 1.0f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.16862746f, 0.16862746f, 0.16862746f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.1254902f, 0.1254902f, 0.1254902f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.2509804f, 0.2509804f, 0.2509804f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.3019608f, 0.3019608f, 0.3019608f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.34901962f, 0.34901962f, 0.34901962f, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 0.47058824f, 0.84313726f, 1.0f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.0f, 0.47058824f, 0.84313726f, 1.0f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.32941177f, 0.6f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.16862746f, 0.16862746f, 0.16862746f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.21568628f, 0.21568628f, 0.21568628f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.2509804f, 0.2509804f, 0.2509804f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.21568628f, 0.21568628f, 0.21568628f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.2509804f, 0.2509804f, 0.2509804f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.3019608f, 0.3019608f, 0.3019608f, 1.0f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.21568628f, 0.21568628f, 0.21568628f, 1.0f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.2509804f, 0.2509804f, 0.2509804f, 1.0f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.3019608f, 0.3019608f, 0.3019608f, 1.0f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.21568628f, 0.21568628f, 0.21568628f, 1.0f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.2509804f, 0.2509804f, 0.2509804f, 1.0f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.3019608f, 0.3019608f, 0.3019608f, 1.0f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.16862746f, 0.16862746f, 0.16862746f, 1.0f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.21568628f, 0.21568628f, 0.21568628f, 1.0f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.2509804f, 0.2509804f, 0.2509804f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.16862746f, 0.16862746f, 0.16862746f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.21568628f, 0.21568628f, 0.21568628f, 1.0f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.0f, 0.47058824f, 0.84313726f, 1.0f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.0f, 0.32941177f, 0.6f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.0f, 0.47058824f, 0.84313726f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.0f, 0.32941177f, 0.6f, 1.0f);
    style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882353f, 0.1882353f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.30980393f, 0.30980393f, 0.34901962f, 1.0f);
    style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.22745098f, 0.22745098f, 0.24705882f, 1.0f);
    style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.06f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.0f, 0.47058824f, 0.84313726f, 1.0f);
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.9f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 1.0f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.35f);
}

class TestImGuiWidget : public Phoenix::UI::ImGuiWidget
{
public:

    void Render() override
    {
        ImGui::Button("Foobar");
    }
};

void InitializeApplication()
{
    // ReSharper disable CppExpressionWithoutSideEffects
    Phoenix::ServiceContainerBuilder serviceContainerBuilder;
    serviceContainerBuilder.RegisterService<Phoenix::UI::MenuManager>().AsInterfaces().InstancePerScope();
    serviceContainerBuilder.RegisterService<Phoenix::UI::ImGuiDockManager>().AsInterfaces().InstancePerScope();

    Phoenix::Editor::CtorArgs editorArgs;
    editorArgs.Builder = &serviceContainerBuilder;

    GEditor = std::make_shared<Phoenix::Editor>(editorArgs);
    GEditor->Initialize();

    if (auto dockManager = GEditor->ResolveService<Phoenix::UI::IDockManager>())
    {
        Phoenix::UI::DockTabFactory factory;
        factory.FactoryFunc = [](const Phoenix::UI::DockTabId& tabId)
        {
            auto dockTab = std::make_shared<Phoenix::UI::ImGuiDockTab>(tabId);

            dockTab->SetContentWidget(std::make_shared<TestImGuiWidget>());

            return dockTab;
        };
        dockManager->RegisterDockTabFactory("TestWindow", factory);
    }

    auto editorWidgetContext = std::make_shared<Phoenix::UI::EditorWidgetContext>();
    editorWidgetContext->Editor = GEditor;

    Phoenix::UI::ImGuiWidgetContext mainFrameContext;
    mainFrameContext.AddObject(editorWidgetContext);

    GMainFrameWidget = std::make_shared<Phoenix::UI::MainFrameWidget>(mainFrameContext);

    // auto path = R"(C:\Users\jlfar\OneDrive\Documents\Projects\Phoenix\PhoenixSim\Tests\TestRTS\Data\Catalogs\Core\Objects\Units\Archer\Archer.json)";
    // std::ifstream file(path);
    // nlohmann::json asdf = nlohmann::json::parse(file);
    //
    // auto jsonModelObject = std::make_shared<Phoenix::JsonModelObject>();
    // jsonModelObject->Id = "Archer";
    // jsonModelObject->Data = asdf;
    // jsonModelObject->DisplayName = "Archer";
    //
    // Phoenix::JsonModelDescriptorBuilder builder;
    // auto rootModelDescriptor = builder.Build(jsonModelObject);
    
}