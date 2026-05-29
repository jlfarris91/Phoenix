#include "SDL3PlatformService.h"

#include "Phoenix/Logging.h"

using namespace Phoenix;
using namespace Phoenix::App::Dev;

SDL3PlatformService::SDL3PlatformService(SDL3WindowArgs args)
    : Args(std::move(args))
{
}

void SDL3PlatformService::Initialize(const std::shared_ptr<Application>& app)
{
    IAppService::Initialize(app);

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        LogError("SDL_Init failed: %s", SDL_GetError());
        return;
    }

    SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;

    Window = SDL_CreateWindow(Args.Title.c_str(), Args.Width, Args.Height, flags);
    if (!Window)
    {
        LogError("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return;
    }

    Renderer = SDL_CreateRenderer(Window, nullptr);
    if (!Renderer)
    {
        LogError("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(Window);
        Window = nullptr;
        SDL_Quit();
        return;
    }

    SDL_SetRenderVSync(Renderer, 1);

    DisplayScale = SDL_GetWindowDisplayScale(Window);
    if (DisplayScale <= 0.0f)
        DisplayScale = 1.0f;
}

void SDL3PlatformService::Shutdown()
{
    if (Renderer)
    {
        SDL_DestroyRenderer(Renderer);
        Renderer = nullptr;
    }
    if (Window)
    {
        SDL_DestroyWindow(Window);
        Window = nullptr;
    }
    SDL_Quit();

    IAppService::Shutdown();
}

void SDL3PlatformService::PollEvents()
{
    auto windowId = SDL_GetWindowID(Window);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        OnEvent.Broadcast(event);

        if (event.type == SDL_EVENT_QUIT)
            QuitFlag = true;

        if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == windowId)
            QuitFlag = true;

        if (event.type == SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED && event.window.windowID == windowId)
        {
            DisplayScale = SDL_GetWindowDisplayScale(Window);
            if (DisplayScale <= 0.0f)
                DisplayScale = 1.0f;
            OnDisplayScaleChanged.Broadcast(DisplayScale);
        }
    }
}

bool SDL3PlatformService::WantsQuit() const
{
    return QuitFlag;
}

SDL_Window* SDL3PlatformService::GetWindow() const
{
    return Window;
}

SDL_Renderer* SDL3PlatformService::GetRenderer() const
{
    return Renderer;
}

float SDL3PlatformService::GetDisplayScale() const
{
    return DisplayScale;
}
