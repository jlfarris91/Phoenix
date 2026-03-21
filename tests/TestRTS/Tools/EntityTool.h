
#pragma once

#include <PhoenixSim/Reflection/Reflection.h>

#include "../SDL/SDLTool.h"

namespace Phoenix
{
    class Session;

    struct EntityTool : ISDLTool
    {
        PHX_DECLARE_TYPE(EntityTool)

        EntityTool(const std::shared_ptr<Session>& session);

        void OnAppRenderWorld(WorldConstRef world, SDLDebugState& state, SDLDebugRenderer& renderer) override;
        void OnAppRenderUI(ImGuiIO& io) override;
        void OnAppEvent(WorldConstRef world, SDLDebugState& state, SDL_Event* event) override;

        std::shared_ptr<Session> Session;
        float BrushSize = 10.0f;
        uint32 SpawnCount = 1;
        uint8 Player = 0;
        float MoveSpeed = 10.0f;
        float PushForce = 100.0f;

        size_t SelectedUnitIndex = 0;
        std::vector<std::string> UnitLabels;
    };
}
