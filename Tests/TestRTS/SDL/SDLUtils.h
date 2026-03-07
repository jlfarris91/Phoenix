
#pragma once

#include <SDL3/SDL_video.h>

#include "PhoenixSim/WorldsFwd.h"
#include "PhoenixSim/ECS/EntityId.h"

namespace Phoenix
{
    struct SDLCamera;
    struct SDLViewport;
    struct SDLDebugRenderer;
}

void DrawGrid(
    SDL_Window* window,
    Phoenix::SDLDebugRenderer* renderer,
    const Phoenix::SDLViewport* viewport,
    const Phoenix::SDLCamera* camera);

void DrawSelectionCircle(
    Phoenix::WorldConstRef world,
    Phoenix::SDLDebugRenderer& renderer,
    Phoenix::ECS::EntityId entityId);