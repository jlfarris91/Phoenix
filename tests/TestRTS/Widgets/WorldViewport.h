#pragma once

#include <imgui.h>
#include <SDL3/SDL_render.h>

#include "../SDL/SDLCamera.h"

#include "WorldViewWidget.h"

struct SDLCamera;
struct SDLViewport;

class WorldViewport : public WorldViewWidget
{
protected:

    WorldViewport();
    ~WorldViewport() override;

    void Render(Phoenix::SessionConstRef session, Phoenix::WorldConstRef world) override;

    SDLCamera& GetCamera();
    const SDLCamera& GetCamera() const;

    SDLViewport* GetViewport() const;

private:

    SDLCamera Camera;
    SDLViewport* Viewport = nullptr;
};
