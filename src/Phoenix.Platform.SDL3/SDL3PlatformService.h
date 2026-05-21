#pragma once

#include <string>

#include <SDL3/SDL.h>

#include "Application/IPlatformService.h"
#include "Phoenix/Delegates.h"

namespace Phoenix
{
    struct SDL3WindowArgs
    {
        std::string Title   = "Phoenix";
        int         Width   = 1280;
        int         Height  = 720;
    };

    PHX_DECLARE_MULTICAST_DELEGATE(FOnSDL3Event, SDL_Event&);
    PHX_DECLARE_MULTICAST_DELEGATE(FOnAfterPollEvents);

    class SDL3PlatformService : public IPlatformService
    {
        PHX_DECLARE_TYPE_DERIVED(SDL3PlatformService, IPlatformService)
    public:

        explicit SDL3PlatformService(SDL3WindowArgs args = {});

        void Initialize(const std::shared_ptr<Application>& app) override;
        void Shutdown() override;

        void PreTick() override;

        bool WantsQuit() const override;

        SDL_Window*   GetWindow()       const;
        SDL_GLContext GetGLContext()    const;
        float         GetDisplayScale() const;

        FOnSDL3Event       OnEvent;
        FOnAfterPollEvents OnAfterPollEvents;

        PHX_DECLARE_MULTICAST_DELEGATE(FOnDisplayScaleChanged, float);
        FOnDisplayScaleChanged OnDisplayScaleChanged;

    private:

        void PollEvents();

        SDL3WindowArgs Args;
        SDL_Window*    Window        = nullptr;
        SDL_GLContext  GLContext     = nullptr;
        bool           QuitFlag      = false;
        float          DisplayScale  = 1.0f;
    };
}
