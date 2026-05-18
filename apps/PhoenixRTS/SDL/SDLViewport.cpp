
#include "SDLViewport.h"

#include <SDL3/SDL_render.h>

#include <Phoenix/Platform.h>

#include "SDLCamera.h"

using namespace Phoenix;

SDLViewport::SDLViewport(SDL_Window* window, SDL_Renderer* renderer, SDLCamera* camera)
    : Window(window)
    , Renderer(renderer)
    , Camera(camera)
{
}

SDL_FPoint SDLViewport::WindowPosToViewportPos(const SDL_FPoint& windowPos) const
{
    return { windowPos.x - Offset.x, windowPos.y - Offset.y };
}

Vec2 SDLViewport::ViewportPosToWorldPos(const SDL_FPoint& pos) const
{
    PHX_ASSERT(Camera);

    int32 w = Width, h = Height;
    if (w <= 0 || h <= 0)
        SDL_GetWindowSize(Window, &w, &h);
    float halfW = (float)(w >> 1);
    float halfH = (float)(h >> 1);

    float posx = (pos.x - halfW) * (1.0f / Camera->Zoom) * (1.0f / Scale.x);
    float posy = (halfH - pos.y) * (1.0f / Camera->Zoom) * (1.0f / Scale.y);

    Vec2 result;
    result.X = posx + Camera->Position.X;
    result.Y = posy + Camera->Position.Y;

    return result;
}

Vec2 SDLViewport::ViewportVecToWorldVec(const SDL_FPoint& vec) const
{
    PHX_ASSERT(Camera);

    Vec2 result;
    result.X = vec.x * (1.0f / Camera->Zoom) * (1.0f / Scale.x);
    result.Y = -vec.y * (1.0f / Camera->Zoom) * (1.0f / Scale.y);

    return result;
}

SDL_FPoint SDLViewport::WorldPosToViewportPos(const Vec2& pos) const
{
    PHX_ASSERT(Camera);

    int32 w = Width, h = Height;
    if (w <= 0 || h <= 0)
        SDL_GetWindowSize(Window, &w, &h);
    float halfW = (float)(w >> 1);
    float halfH = (float)(h >> 1);

    float posx = (float)pos.X - (float)Camera->Position.X;
    float posy = (float)pos.Y - (float)Camera->Position.Y;

    posx *= Camera->Zoom;
    posy *= Camera->Zoom;
    posx *= Scale.x;
    posy *= Scale.y;

    posx += halfW;
    posy = -(posy - halfH);

    SDL_FPoint result;
    result.x = posx;
    result.y = posy;

    return result;
}

SDL_FPoint SDLViewport::WorldVecToViewportVec(const Vec2& vec) const
{
    PHX_ASSERT(Camera);

    SDL_FPoint result;
    result.x = (float)vec.X * Camera->Zoom;
    result.y = -(float)vec.Y * Camera->Zoom;
    result.x *= Scale.x;
    result.y *= Scale.y;

    return result;
}
