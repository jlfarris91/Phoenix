#pragma once

#include <SDL3/SDL.h>

#include "IImGuiService.h"
#include "Phoenix/Delegates.h"

namespace Phoenix::UI
{
    class SDL3ImGuiService : public IImGuiService
    {
        PHX_DECLARE_TYPE_DERIVED(SDL3ImGuiService, IImGuiService)
    public:

        void Initialize(const std::shared_ptr<Application>& app) override;
        void Shutdown() override;
        void Tick() override;
        void PostTick() override;

    private:

        void HandleEvent(SDL_Event& event);
        void BeginFrame();
        void OnDisplayScaleChanged(float scale);

        static void SetupStyle(float dpiScale);

        DelegateHandle EventHandle;
        DelegateHandle AfterPollHandle;
        DelegateHandle DisplayScaleHandle;
    };
}
