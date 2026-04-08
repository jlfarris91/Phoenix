

#pragma once

#include <imgui_impl_sdl3.h>

#include <PhoenixSim/WorldsFwd.h>
#include <PhoenixSim/Reflection/Registration.h>

struct SDLDebugRenderer;
struct SDLDebugState;

struct ISDLTool
{
    PHX_DECLARE_TYPE_INTERFACE(ISDLTool)

    virtual ~ISDLTool() = default;

    virtual const char* GetDescription() const { return ""; }

    virtual void OnActivated() {}
    virtual void OnDeactivated() {}
    virtual void OnAppRenderWorld(Phoenix::WorldConstRef world, SDLDebugState& state, SDLDebugRenderer& renderer) {}
    virtual void OnAppRenderUI(ImGuiIO& io) {}
    virtual void OnAppEvent(Phoenix::WorldConstRef world, SDLDebugState& state, SDL_Event* event) {}
};