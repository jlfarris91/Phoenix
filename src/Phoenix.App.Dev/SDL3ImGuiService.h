#pragma once

#include <SDL3/SDL.h>

#include "IImGuiService.h"
#include "Phoenix/Delegates.h"

namespace Phoenix::App::Dev
{
    class SDL3Renderer;
    class SDL3PlatformService;

    class SDL3ImGuiService : public UI::IImGuiService
    {
        PHX_DECLARE_TYPE_DERIVED(SDL3ImGuiService, IImGuiService)
    public:

        void Initialize(const std::shared_ptr<Application>& app) override;
        void Shutdown() override;

        void BeginFrame();
        void EndFrame();

    private:

        void HandleEvent(SDL_Event& event);
        void OnDisplayScaleChanged(float scale);

        static void SetupStyle(float dpiScale);

        std::shared_ptr<SDL3PlatformService> Platform;
        SDL_Renderer* Renderer = nullptr;

        DelegateHandle EventHandle;
        DelegateHandle AfterPollHandle;
        DelegateHandle DisplayScaleHandle;
    };
}
