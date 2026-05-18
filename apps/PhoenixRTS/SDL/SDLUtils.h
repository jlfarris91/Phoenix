
#pragma once

#include <SDL3/SDL_video.h>

#include "Phoenix.Sim/WorldsFwd.h"
#include "Phoenix.Sim.ECS/EntityId.h"

struct SDLCamera;
struct SDLViewport;
struct SDLDebugRenderer;

void DrawGrid(
    SDL_Window* window,
    SDLDebugRenderer* renderer,
    const SDLViewport* viewport,
    const SDLCamera* camera);

void DrawSelectionCircle(
    Phoenix::WorldConstRef world,
    SDLDebugRenderer& renderer,
    Phoenix::ECS::EntityId entityId);