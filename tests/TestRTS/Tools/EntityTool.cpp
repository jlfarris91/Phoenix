
#include "EntityTool.h"

#include <SDL3/SDL_events.h>

#include <PhoenixSim/Actions.h>
#include <PhoenixSim/Session.h>
#include <PhoenixSim/FixedPoint/FixedVector.h>

#include "../ImGuiUtils.h"
#include "../SDL/SDLDebugState.h"
#include "../SDL/SDLDebugRenderer.h"

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
        action.Data[0].Name = UnitLabels[SelectedUnitIndex];
        action.Data[1].Distance = mouseWorldPos.X;
        action.Data[2].Distance = mouseWorldPos.Y;
        action.Data[3].Degrees = Vec2::RandUnitVector().AsRadians();
        action.Data[4].UInt32 = SpawnCount;
        action.Data[5].UInt32 = Player;
        Session->EnqueueAction(action);
    }

    // Release entities
    if (state.KeyDown(SDLK_X))
    {
        Action action;
        action.Verb = "release_entities_in_range"_n;
        action.Data[0].Distance = mouseWorldPos.X;
        action.Data[1].Distance = mouseWorldPos.Y;
        action.Data[2].Distance = BrushSize;
        Session->EnqueueAction(action);
    }

    // Push entities
    if (state.KeyDown(SDLK_F))
    {
        Action action;
        action.Verb = "push_entities_in_range"_n;
        action.Data[0].Distance = mouseWorldPos.X;
        action.Data[1].Distance = mouseWorldPos.Y;
        action.Data[2].Distance = BrushSize;
        action.Data[3].Value = PushForce;
        Session->EnqueueAction(action);
    }
}
