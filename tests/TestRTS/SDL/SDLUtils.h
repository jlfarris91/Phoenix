
#pragma once

#include <SDL3/SDL_video.h>

#include "PhoenixSim/WorldsFwd.h"
#include "PhoenixSim/ECS/EntityId.h"

struct SDLCamera;
struct SDLViewport;
struct SDLDebugRenderer;

void DrawGrid(
    SDL_Rect rect,
    SDLDebugRenderer& renderer,
    const SDLViewport& viewport,
    const SDLCamera& camera);

void DrawSelectionCircle(
    Phoenix::WorldConstRef world,
    SDLDebugRenderer& renderer,
    Phoenix::ECS::EntityId entityId);