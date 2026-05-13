
#include "CameraTool.h"

#include <SDL3/SDL_events.h>

#include <PhoenixSim/Session.h>
#include <PhoenixSim/FixedPoint/FixedVector.h>

#include "../sdl/SDLCamera.h"
#include "../sdl/SDLDebugState.h"
#include "../sdl/SDLDebugRenderer.h"

using namespace Phoenix;

const char* CameraTool::GetDescription() const
{
    return "Pan (right-drag or arrow keys) and zoom (scroll wheel) the game viewport camera.";
}

CameraTool::CameraTool(const std::shared_ptr<Phoenix::Session>& session, SDLCamera* camera, SDLViewport* viewport)
    : Session(session)
    , Camera(camera)
    , Viewport(viewport)
{
}

void CameraTool::OnAppEvent(WorldConstRef, IDebugState& state, const void* eventData)
{
    float mx, my;
    SDL_GetMouseState(&mx, &my);
    SDL_FPoint mouseWindowPos = { mx, my };
    Vec2 mouseWorldPos = state.GetWorldMousePos();

    const SDL_Event* event = static_cast<const SDL_Event*>(eventData);

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
