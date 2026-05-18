
#pragma once

#include <SDL3/SDL_video.h>

#include <Phoenix/FixedPoint/FixedVector.h>

struct SDL_Renderer;
struct SDLCamera;

struct SDLViewport
{
    SDLViewport(SDL_Window* window, SDL_Renderer* renderer, SDLCamera* camera);

    // Convert an OS window-space position to viewport-local coordinates
    SDL_FPoint WindowPosToViewportPos(const SDL_FPoint& windowPos) const;

    Phoenix::Vec2 ViewportPosToWorldPos(const SDL_FPoint& pos) const;
    Phoenix::Vec2 ViewportVecToWorldVec(const SDL_FPoint& vec) const;

    SDL_FPoint WorldPosToViewportPos(const Phoenix::Vec2& pos) const;
    SDL_FPoint WorldVecToViewportVec(const Phoenix::Vec2& vec) const;

    SDL_Window* Window;
    SDL_Renderer* Renderer;
    SDLCamera* Camera;

    // Game viewport region within the OS window (updated each frame from ImGui)
    SDL_FPoint Offset = { 0.0f, 0.0f };
    int Width = 0;
    int Height = 0;
    SDL_FPoint Scale = { 1.0f, 1.0f };
};