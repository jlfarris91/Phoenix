
#pragma once

#include <SDL3/SDL_rect.h>

#include <PhoenixSim/Containers/Optional.h>
#include <PhoenixSim/Reflection/Reflection.h>

#include "../SDL/SDLCamera.h"
#include "../SDL/SDLTool.h"

namespace Phoenix
{
    struct SDLViewport;
    struct SDLCamera;
    class Session;

    struct CameraTool : ISDLTool
    {
        PHX_DECLARE_TYPE(CameraTool)

        CameraTool(std::shared_ptr<Session> session, SDLCamera* camera, SDLViewport* viewport);

        void OnAppRenderWorld(WorldConstRef world, SDLDebugState& state, SDLDebugRenderer& renderer) override;
        void OnAppRenderUI(ImGuiIO& io) override;
        void OnAppEvent(WorldConstRef world, SDLDebugState& state, SDL_Event* event) override;

        std::shared_ptr<Session> Session;
        SDLCamera* Camera;
        SDLViewport* Viewport;
        TOptional<SDL_FPoint> CameraDragPos;
        float PanSpeed = 100.0f;
        float ZoomSpeed = 0.1f;
    };
}