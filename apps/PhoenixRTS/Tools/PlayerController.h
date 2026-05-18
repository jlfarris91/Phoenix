
#pragma once

#include <SDL3/SDL_rect.h>

#include <Phoenix.Sim/Containers/Optional.h>
#include <Phoenix.Sim.ECS/EntityId.h>

#include "../sdl/SDLCamera.h"
#include "../sdl/SDLTool.h"

namespace Phoenix
{
    class Session;
}

struct SDLViewport;
struct SDLCamera;

struct PlayerController : public ISDLTool
{
    PHX_DECLARE_TYPE_DERIVED(PlayerController, ISDLTool)

    const char* GetDescription() const override { return "Select and issue orders to units. Left-click to select, right-click to move or attack."; }

    PlayerController(const std::shared_ptr<Phoenix::Session>& session, SDLCamera* camera, SDLViewport* viewport);

    void OnActivated() override;
    void OnDeactivated() override;
    void OnAppRenderWorld(Phoenix::WorldConstRef world, SDLDebugState& state, SDLDebugRenderer& renderer) override;
    void OnAppRenderUI(ImGuiIO& io) override;
    void OnAppEvent(Phoenix::WorldConstRef world, SDLDebugState& state, SDL_Event* event) override;

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