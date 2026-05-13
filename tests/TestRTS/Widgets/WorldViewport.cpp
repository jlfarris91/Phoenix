#include "WorldViewport.h"

#include "../SDL/SDLCamera.h"
#include "../SDL/SDLViewport.h"

#include <imgui.h>

WorldViewport::WorldViewport()
{
    Viewport = new SDLViewport(nullptr, nullptr, nullptr);
}

WorldViewport::~WorldViewport()
{
    delete Viewport;
    Viewport = nullptr;

    // if (WorldRenderTexture)
    // {
    //     SDL_DestroyTexture(WorldRenderTexture);
    //     WorldRenderTexture = nullptr;
    // }
}

void WorldViewport::Render(Phoenix::SessionConstRef session, Phoenix::WorldConstRef world)
{
    ImGui::Text("WORLD VIEWPORT");
    
    // GGameWindowHovered = ImGui::IsWindowHovered();
    // ImVec2 contentSize = ImGui::GetContentRegionAvail();
    // if (contentSize.x > 0 && contentSize.y > 0)
    // {
    //     WorldRenderSize = contentSize;
    //     ImVec2 contentPos = ImGui::GetCursorScreenPos();
    //     Viewport->Offset = { contentPos.x, contentPos.y };
    //     Viewport->Width  = (int)contentSize.x;
    //     Viewport->Height = (int)contentSize.y;
    //     if (WorldRenderTexture)
    //     {
    //         ImGui::Image((ImTextureID)(intptr_t)WorldRenderTexture, contentSize);
    //     }
    // }
}

SDLCamera& WorldViewport::GetCamera()
{
    return Camera;
}

const SDLCamera& WorldViewport::GetCamera() const
{
    return Camera;
}

SDLViewport* WorldViewport::GetViewport() const
{
    return Viewport;
}
