
#pragma once

#include "../sdl/SDLTool.h"
#include "PhoenixSim/Reflection/Registration.h"

namespace Phoenix
{
    class Session;
}

struct EntityTool : public ISDLTool
{
    PHX_DECLARE_TYPE_DERIVED(EntityTool, ISDLTool)

    const char* GetDescription() const override { return "Spawn and manipulate entities in the world using brush-based placement."; }

    EntityTool(const std::shared_ptr<Phoenix::Session>& session);

    void OnAppRenderWorld(Phoenix::WorldConstRef world, SDLDebugState& state, SDLDebugRenderer& renderer) override;
    void OnAppRenderUI(ImGuiIO& io) override;
    void OnAppEvent(Phoenix::WorldConstRef world, SDLDebugState& state, SDL_Event* event) override;

    std::shared_ptr<Phoenix::Session> Session;
    float BrushSize = 10.0f;
    uint32_t SpawnCount = 1;
    uint8_t Player = 0;
    bool RandomPlayer = false;
    float MoveSpeed = 10.0f;
    float PushForce = 100.0f;

    size_t SelectedUnitIndex = 0;
    std::vector<std::string> UnitLabels;
};

PHX_DEFINE_TYPE(EntityTool)
{
    registration
        .Field("BrushSize", &EntityTool::BrushSize)
        .Field("SpawnCount", &EntityTool::SpawnCount)
        .Field("Player", &EntityTool::Player)
        .Field("RandomPlayer", &EntityTool::RandomPlayer)
        .Field("MoveSpeed", &EntityTool::MoveSpeed)
        .Field("PushForce", &EntityTool::PushForce);
}