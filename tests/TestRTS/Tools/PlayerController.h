
#pragma once

#include <SDL3/SDL_rect.h>

#include <PhoenixSim/Containers/Optional.h>
#include <PhoenixSim/ECS/EntityId.h>

#include "../sdl/SDLCamera.h"
#include "../App/Tool.h"

namespace Phoenix
{
    class Session;
}

struct SDLViewport;
struct SDLCamera;

class PlayerController : public ITool
{
    PHX_DECLARE_TYPE_DERIVED(PlayerController, ITool)

public:

    const char* GetDescription() const override { return "Select and issue orders to units. Left-click to select, right-click to move or attack."; }

    PlayerController(const std::shared_ptr<Phoenix::Session>& session, SDLCamera* camera, SDLViewport* viewport);

    void OnActivated() override;
    void OnDeactivated() override;
    void OnAppRenderWorld(Phoenix::WorldConstRef world, Phoenix::IDebugState& state, Phoenix::IDebugRenderer& renderer) override;
    void OnAppEvent(Phoenix::WorldConstRef world, Phoenix::IDebugState& state, const void* eventData) override;

private:

    std::shared_ptr<Phoenix::Session> Session;
    SDLCamera* Camera;
    SDLViewport* Viewport;
    float PanSpeed = 100.0f;
    float ZoomSpeed = 0.1f;

    Phoenix::Vec2 CursorWorldPos;
    Phoenix::TOptional<SDL_FPoint> CameraDragPos;
    Phoenix::TOptional<SDL_FPoint> CursorDragStart;
    Phoenix::TOptional<SDL_FPoint> BoxSelectDragStart, BoxSelectDragEnd;
    Phoenix::ECS::EntityId HoverTarget;
};

PHX_DEFINE_TYPE(PlayerController)
{
    registration
        .Field("PanSpeed", &PlayerController::PanSpeed)
        .Field("ZoomSpeed", &PlayerController::ZoomSpeed);
}