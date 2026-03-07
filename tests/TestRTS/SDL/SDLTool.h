

#pragma once

#include "imgui_impl_sdl3.h"

#include <PhoenixSim/WorldsFwd.h>

namespace Phoenix
{
    struct SDLDebugRenderer;
    struct SDLDebugState;

    struct ISDLTool
    {
        virtual ~ISDLTool() = default;

        virtual const TypeDescriptor& GetTypeDescriptor() const = 0;

        virtual void OnActivated() {}
        virtual void OnDeactivated() {}
        virtual void OnAppRenderWorld(WorldConstRef world, SDLDebugState& state, SDLDebugRenderer& renderer) {}
        virtual void OnAppRenderUI(ImGuiIO& io) {}
        virtual void OnAppEvent(WorldConstRef world, SDLDebugState& state, SDL_Event* event) {}
    };
}
