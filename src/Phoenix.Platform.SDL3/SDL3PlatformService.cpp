#include "SDL3PlatformService.h"

#include <cstdio>

using namespace Phoenix;

SDL3PlatformService::SDL3PlatformService(SDL3WindowArgs args)
    : Args(std::move(args))
{
}

void SDL3PlatformService::Initialize(const std::shared_ptr<Application>& app)
{
    IAppService::Initialize(app);

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return;
    }

    SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;

    if (Args.UseOpenGL)
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,         0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,  SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,          1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,            24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,          8);
        flags |= SDL_WINDOW_OPENGL;
    }

    Window = SDL_CreateWindow(Args.Title.c_str(), Args.Width, Args.Height, flags);
    if (!Window)
    {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return;
    }

    if (Args.UseOpenGL)
    {
        GLContext = SDL_GL_CreateContext(Window);
        if (!GLContext)
        {
            SDL_Log("SDL_GL_CreateContext failed: %s", SDL_GetError());
            SDL_DestroyWindow(Window);
            Window = nullptr;
            SDL_Quit();
            return;
        }
        SDL_GL_MakeCurrent(Window, GLContext);
        SDL_GL_SetSwapInterval(1);
    }
}

void SDL3PlatformService::Shutdown()
{
    if (Args.UseOpenGL && GLContext)
    {
        SDL_GL_DestroyContext(GLContext);
        GLContext = nullptr;
    }
    if (Window)
    {
        SDL_DestroyWindow(Window);
        Window = nullptr;
    }
    SDL_Quit();

    IAppService::Shutdown();
}

void SDL3PlatformService::PreTick()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        OnEvent.Broadcast(event);

        if (event.type == SDL_EVENT_QUIT)
            QuitFlag = true;
        if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
            event.window.windowID == SDL_GetWindowID(Window))
            QuitFlag = true;
    }

    OnAfterPollEvents.Broadcast();
}

bool SDL3PlatformService::WantsQuit() const
{
    return QuitFlag;
}

SDL_Window* SDL3PlatformService::GetWindow() const
{
    return Window;
}

SDL_GLContext SDL3PlatformService::GetGLContext() const
{
    return GLContext;
}
