
#pragma once

#include <SDL3/SDL_rect.h>

#include <Phoenix.Sim/Containers/Optional.h>

#include "../sdl/SDLCamera.h"
#include "../sdl/SDLTool.h"

namespace Phoenix
{
    class Session;
}

struct SDLViewport;
struct SDLCamera;

struct CameraTool : public ISDLTool
{
    PHX_DECLARE_TYPE_DERIVED(CameraTool, ISDLTool)

    const char* GetDescription() const override { return "Pan (right-drag or arrow keys) and zoom (scroll wheel) the game viewport camera."; }

    CameraTool(std::shared_ptr<Phoenix::Session> session, SDLCamera* camera, SDLViewport* viewport);

    void OnAppRenderWorld(Phoenix::WorldConstRef world, SDLDebugState& state, SDLDebugRenderer& renderer) override;
    void OnAppRenderUI(ImGuiIO& io) override;
    void OnAppEvent(Phoenix::WorldConstRef world, SDLDebugState& state, SDL_Event* event) override;

    std::shared_ptr<Phoenix::Session> Session;
    SDLCamera* Camera;
    SDLViewport* Viewport;
    Phoenix::TOptional<SDL_FPoint> CameraDragPos;
    float PanSpeed = 100.0f;
    float ZoomSpeed = 0.1f;
};

PHX_DEFINE_TYPE(CameraTool)
{
    registration
        .Field("PanSpeed", &CameraTool::PanSpeed)
        .Field("ZoomSpeed", &CameraTool::ZoomSpeed);
}