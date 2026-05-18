
#include "CameraTool.h"

#include <SDL3/SDL_events.h>

#include <Phoenix.Sim/Session.h>
#include <Phoenix.Sim/FixedPoint/FixedVector.h>

#include "../sdl/SDLCamera.h"
#include "../sdl/SDLDebugState.h"
#include "../sdl/SDLDebugRenderer.h"

using namespace Phoenix;

CameraTool::CameraTool(std::shared_ptr<Phoenix::Session> session, SDLCamera* camera, SDLViewport* viewport)
    : Session(session)
    , Camera(camera)
    , Viewport(viewport)
{
}

void CameraTool::OnAppRenderWorld(WorldConstRef world, SDLDebugState& state, SDLDebugRenderer& renderer)
{
}

void CameraTool::OnAppRenderUI(ImGuiIO& io)
{
}

void CameraTool::OnAppEvent(WorldConstRef world, SDLDebugState& state, SDL_Event* event)
{
    float mx, my;
    SDL_GetMouseState(&mx, &my);
    SDL_FPoint mouseWindowPos = { mx, my };
    Vec2 mouseWorldPos = state.GetWorldMousePos();

    if (event->type == SDL_EVENT_MOUSE_MOTION)
    {
        if (CameraDragPos.IsSet())
        {
            Vec2 lastMouseWorldPos = Viewport->ViewportPosToWorldPos(Viewport->WindowPosToViewportPos(*CameraDragPos));
            Vec2 mouseDelta = mouseWorldPos - lastMouseWorldPos;
            Camera->Position -= mouseDelta;
            CameraDragPos = mouseWindowPos;
        }
    }

    if (event->type == SDL_EVENT_MOUSE_BUTTON_UP)
    {
        if (event->button.button == SDL_BUTTON_RIGHT)
        {
            CameraDragPos.Reset();
        }
    }

    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
        if (event->button.button == SDL_BUTTON_RIGHT)
        {
            CameraDragPos = mouseWindowPos;
        }
    }

    if (event->type == SDL_EVENT_KEY_DOWN)
    {
        if (event->key.key == SDLK_LEFT)
        {
            Camera->Position.X -= PanSpeed;
        }

        if (event->key.key == SDLK_RIGHT)
        {
            Camera->Position.X += PanSpeed;
        }

        if (event->key.key == SDLK_UP)
        {
            Camera->Position.Y += PanSpeed;
        }

        if (event->key.key == SDLK_DOWN)
        {
            Camera->Position.Y -= PanSpeed;
        }
    }

    if (event->type == SDL_EVENT_KEY_UP)
    {
    }

    if (event->type == SDL_EVENT_MOUSE_WHEEL)
    {
        float zoomScale = 1.0f + event->wheel.y * ZoomSpeed;
        Camera->Zoom = Max(Camera->Zoom * zoomScale, 0.001f);
    }
}
