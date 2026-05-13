#pragma once

#include <imgui.h>
#include <SDL3/SDL_render.h>

#include "PhoenixSim/FPSCalc.h"
#include "PhoenixSim/WorldsFwd.h"

struct SDLDebugRenderer;
struct SDLCamera;
struct SDLViewport;

class SDLWorldRenderer
{
public:
    SDLWorldRenderer(SDL_Renderer* renderer);
    virtual ~SDLWorldRenderer() = default;

    ImVec2 GetRenderSize() const;
    void SetRenderSize(ImVec2 size);

    ImVec4 GetRenderTargetClearColor() const;
    void SetRenderTargetClearColor(ImVec4 color);

    virtual void Render(
        Phoenix::WorldConstRef world,
        const SDLViewport& viewport,
        const SDLCamera& camera,
        SDLDebugRenderer& debugRenderer);

private:

    void ResizeRenderTarget();

    void RenderWorld(Phoenix::WorldConstRef world);

    SDL_Renderer* Renderer = nullptr;
    SDL_Texture* WorldRenderTexture = nullptr;
    int WorldRenderTextureW = 0;
    int WorldRenderTextureH = 0;
    ImVec2 WorldRenderSize = { 800.0f, 600.0f };
    ImVec4 RenderTargetClearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    Phoenix::FPSCalc RendererFPS;
    bool bDrawGrid = false;
};
