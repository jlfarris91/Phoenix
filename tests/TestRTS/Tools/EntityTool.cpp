
#include "EntityTool.h"

#include <SDL3/SDL_events.h>

#include <Phoenix.Sim/Actions.h>
#include <Phoenix.Sim/Session.h>
#include <Phoenix.Sim/FixedPoint/FixedVector.h>

#include "../imgui/ImGuiUtils.h"
#include "../sdl/SDLDebugState.h"
#include "../sdl/SDLDebugRenderer.h"

using namespace Phoenix;

EntityTool::EntityTool(const std::shared_ptr<Phoenix::Session>& session)
    : Session(session)
{
    UnitLabels = { "Lancer", "Archer", "Tower" };
}

void EntityTool::OnAppRenderWorld(WorldConstRef world, SDLDebugState& state, SDLDebugRenderer& renderer)
{
    Vec2 mouseWorldPos = state.GetWorldMousePos();

    if (state.KeyDown(SDLK_X))
    {
        renderer.DrawCircle(mouseWorldPos, BrushSize, Color::White);
    }

    if (state.KeyDown(SDLK_F))
    {
        renderer.DrawCircle(mouseWorldPos, BrushSize, Color::White);
    }
}

void EntityTool::OnAppRenderUI(ImGuiIO& io)
{
    ImGui::TextDisabled("Units:");
    ImGui::Spacing();
    WrapPanel(UnitLabels, SelectedUnitIndex);
}

void EntityTool::OnAppEvent(WorldConstRef world, SDLDebugState& state, SDL_Event* event)
{
    Vec2 mouseWorldPos = state.GetWorldMousePos();

    // Spawn entities
    if (state.MouseButtonDown(SDL_BUTTON_LEFT) && SelectedUnitIndex < UnitLabels.size())
    {
        Action action;
        action.Verb = "spawn_entity"_n;
        action.Args[0].AsName = UnitLabels[SelectedUnitIndex];
        action.Args[1].AsDistance = mouseWorldPos.X;
        action.Args[2].AsDistance = mouseWorldPos.Y;
        action.Args[3].AsDegrees = Vec2::RandUnitVector().AsRadians();
        action.Args[4].AsUInt32 = SpawnCount;
        action.Args[5].AsUInt32 = RandomPlayer ? (std::rand() % 8) : Player;
        Session->EnqueueAction(action);
    }

    // Release entities
    if (state.KeyDown(SDLK_X))
    {
        Action action;
        action.Verb = "release_entities_in_range"_n;
        action.Args[0].AsDistance = mouseWorldPos.X;
        action.Args[1].AsDistance = mouseWorldPos.Y;
        action.Args[2].AsDistance = BrushSize;
        Session->EnqueueAction(action);
    }

    // Push entities
    if (state.KeyDown(SDLK_F))
    {
        Action action;
        action.Verb = "push_entities_in_range"_n;
        action.Args[0].AsDistance = mouseWorldPos.X;
        action.Args[1].AsDistance = mouseWorldPos.Y;
        action.Args[2].AsDistance = BrushSize;
        action.Args[3].AsValue = PushForce;
        Session->EnqueueAction(action);
    }
}
