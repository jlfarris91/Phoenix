
#pragma once

#include <SDL3/SDL_rect.h>

#include <PhoenixSim/Containers/Optional.h>

#include "../sdl/SDLCamera.h"
#include "../App/Tool.h"

namespace Phoenix
{
    class Session;
}

struct SDLViewport;
struct SDLCamera;

class CameraTool : public ITool
{
    PHX_DECLARE_TYPE_DERIVED(CameraTool, ITool)

public:

    const char* GetDescription() const override;

    CameraTool(const std::shared_ptr<Phoenix::Session>& session, SDLCamera* camera, SDLViewport* viewport);

    void OnAppEvent(Phoenix::WorldConstRef world, Phoenix::IDebugState& state, const void* eventData) override;

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