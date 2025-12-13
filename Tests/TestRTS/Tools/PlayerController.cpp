
#include "PlayerController.h"

#include <SDL3/SDL_events.h>

#include "FeatureECS.h"
#include "FixedPoint/FixedVector.h"
#include "Session.h"
#include "../SDL/SDLCamera.h"
#include "../SDL/SDLDebugState.h"
#include "../SDL/SDLDebugRenderer.h"
#include "Commands/Commands.h"

#include "Selection/FeatureSelection.h"
#include "Orders/FeatureOrderQueue.h"
#include "Units/UnitId.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

PlayerController::PlayerController(const TSharedPtr<Phoenix::Session>& session, SDLCamera* camera, SDLViewport* viewport)
    : Session(session)
    , Camera(camera)
    , Viewport(viewport)
{
}

void PlayerController::OnActivated()
{
    ISDLTool::OnActivated();
    SDL_CaptureMouse(true);
}

void PlayerController::OnDeactivated()
{
    ISDLTool::OnDeactivated();
    SDL_CaptureMouse(false);
}

void PlayerController::OnAppRenderWorld(WorldConstRef world, SDLDebugState& state, SDLDebugRenderer& renderer)
{
    Vec2 mouseWorldPos = state.GetWorldMousePos();

    CursorWorldPos = mouseWorldPos;

    constexpr uint32 playerId = 0;
    const FName groupId = FName::None;

    if (BoxSelectDragStart.IsSet() && BoxSelectDragEnd.IsSet())
    {
        Vec2 boxSelectDragStartWS = state.Viewport->ViewportPosToWorldPos(*BoxSelectDragStart);
        Vec2 boxSelectDragEndWS = state.Viewport->ViewportPosToWorldPos(*BoxSelectDragEnd);

        Vec2 min;
        min.X = std::min(boxSelectDragStartWS.X, boxSelectDragEndWS.X);
        min.Y = std::min(boxSelectDragStartWS.Y, boxSelectDragEndWS.Y);

        Vec2 max;
        max.X = std::max(boxSelectDragStartWS.X, boxSelectDragEndWS.X);
        max.Y = std::max(boxSelectDragStartWS.Y, boxSelectDragEndWS.Y);

        renderer.DrawRect(min, max, Color::White);

        TArray<EntityTransform> entities;
        FeatureECS::QueryEntitiesInRect(world, min, max, entities);

        for (const EntityTransform& entity : entities)
        {
            renderer.DrawCircle(entity.TransformComponent->Transform.Position, 1.0, Color::Green);
        }
    }
    else
    {
        TArray<EntityTransform> entities;
        FeatureECS::QueryEntitiesInRange(world, CursorWorldPos, 1.0, entities);

        for (const EntityTransform& entity : entities)
        {
            renderer.DrawCircle(entity.TransformComponent->Transform.Position, 1.0, Color::Green);
            break;
        }
    }

    EntityId selectionGroup = FeatureSelection::GetPlayerSelection(world, playerId, groupId);
    FeatureECS::ForEachEntityInGroup(world, selectionGroup, [&](const EntityId& entityId)
    {
        auto transformPtr = FeatureECS::GetWorldTransformPtr(world, entityId);
        if (!transformPtr)
        {
            return;
        }

        renderer.DrawCircle(transformPtr->Position, 1.0, Color::Green);

        Vec2 lastTargetPos = transformPtr->Position;
        FeatureOrderQueue::ForEachOrder(world, { entityId }, [&](const Order& order)
        {
            renderer.DrawLine(lastTargetPos, order.Location, Color::Blue);
            lastTargetPos = order.Location;
        });
    });
}

void PlayerController::OnAppRenderUI(ImGuiIO& io)
{
}

void PlayerController::OnAppEvent(WorldConstRef world, SDLDebugState& state, SDL_Event* event)
{
    float mx, my;
    SDL_GetMouseState(&mx, &my);
    SDL_FPoint mouseWindowPos = { mx, my };
    Vec2 mouseWorldPos = state.GetWorldMousePos();

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

    if (event->type == SDL_EVENT_MOUSE_WHEEL)
    {
        float zoomScale = 1.0f + (float)event->wheel.integer_y * ZoomSpeed;
        Camera->Zoom = Max(Camera->Zoom * zoomScale, 0.001f);
    }

    constexpr uint32 playerId = 0;
    const FName groupId = FName::None;

    if (state.MouseButtonPressed(SDL_BUTTON_LEFT) || state.MouseButtonPressed(SDL_BUTTON_RIGHT))
    {
        CursorDragStart = mouseWindowPos;
    }

    auto distance = [](SDL_FPoint a, SDL_FPoint b)
    {
        if (a.x == b.x && a.y == b.y)
            return 0.0f;
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return sqrt(dx*dx + dy*dy);
    };

    const double DragThreshold = 3.0;

    if (event->type == SDL_EVENT_MOUSE_MOTION && CursorDragStart.IsSet())
    {
        if (state.MouseButtonDown(SDL_BUTTON_RIGHT))
        {
            if (!CameraDragPos.IsSet() && distance(mouseWindowPos, *CursorDragStart) > DragThreshold)
            {
                CameraDragPos = mouseWindowPos;
            }
            else if (CameraDragPos.IsSet())
            {
                Vec2 lastMouseWorldPos = Viewport->ViewportPosToWorldPos(*CameraDragPos);
                Vec2 mouseDelta = mouseWorldPos - lastMouseWorldPos;
                Camera->Position -= mouseDelta;
                CameraDragPos = mouseWindowPos;
            }
        }
        else if (state.MouseButtonDown(SDL_BUTTON_LEFT))
        {
            if (!BoxSelectDragStart.IsSet() && distance(mouseWindowPos, *CursorDragStart) > DragThreshold)
            {
                BoxSelectDragStart = CursorDragStart;
            }
            else if (BoxSelectDragStart.IsSet())
            {
                BoxSelectDragEnd = mouseWindowPos;
            }
        }
    }

    if (state.MouseButtonReleased(SDL_BUTTON_LEFT) && !CameraDragPos.IsSet())
    {
        if (BoxSelectDragStart.IsSet() && BoxSelectDragEnd.IsSet() && distance(*BoxSelectDragStart, *BoxSelectDragEnd) > DragThreshold)
        {
            Vec2 boxSelectDragStartWS = state.Viewport->ViewportPosToWorldPos(*BoxSelectDragStart);
            Vec2 boxSelectDragEndWS = state.Viewport->ViewportPosToWorldPos(*BoxSelectDragEnd);

            Vec2 min;
            min.X = std::min(boxSelectDragStartWS.X, boxSelectDragEndWS.X);
            min.Y = std::min(boxSelectDragStartWS.Y, boxSelectDragEndWS.Y);

            Vec2 max;
            max.X = std::max(boxSelectDragStartWS.X, boxSelectDragEndWS.X);
            max.Y = std::max(boxSelectDragStartWS.Y, boxSelectDragEndWS.Y);

            TArray<EntityTransform> entities;
            FeatureECS::QueryEntitiesInRect(world, min, max, entities);

            if (!state.KeyDown(SDL_KMOD_CTRL) && !state.KeyDown(SDL_KMOD_SHIFT))
            {
                Action clearAction;
                clearAction.Verb = "player_selection_clear"_n;
                clearAction.Data[0].UInt32 = playerId;
                clearAction.Data[1].UInt32 = (uint32)groupId;
                Session->EnqueueAction(clearAction);
            }

            for (const EntityTransform& entity : entities)
            {
                FName verb = state.KeyDown(SDL_KMOD_CTRL)
                    ? "player_selection_remove"_n
                    : "player_selection_add"_n;

                Action clearAction;
                clearAction.Verb = verb;
                clearAction.Data[0].UInt32 = playerId;
                clearAction.Data[1].UInt32 = (uint32)groupId;
                clearAction.Data[2].UInt32 = entity.EntityId;
                Session->EnqueueAction(clearAction);
            }
        }
        else
        {
            TArray<EntityTransform> entities;
            FeatureECS::QueryEntitiesInRange(world, CursorWorldPos, 1.0, entities);

            if (!state.KeyDown(SDL_KMOD_CTRL) && !state.KeyDown(SDL_KMOD_SHIFT))
            {
                Action clearAction;
                clearAction.Verb = "player_selection_clear"_n;
                clearAction.Data[0].UInt32 = playerId;
                clearAction.Data[1].UInt32 = (uint32)groupId;
                Session->EnqueueAction(clearAction);
            }

            for (const EntityTransform& entity : entities)
            {
                FName verb = state.KeyDown(SDL_KMOD_CTRL)
                    ? "player_selection_remove"_n
                    : "player_selection_add"_n;

                Action clearAction;
                clearAction.Verb = verb;
                clearAction.Data[0].UInt32 = playerId;
                clearAction.Data[1].UInt32 = (uint32)groupId;
                clearAction.Data[2].UInt32 = entity.EntityId;
                Session->EnqueueAction(clearAction);
                break;
            }
        }

        CursorDragStart.Reset();
        BoxSelectDragStart.Reset();
        BoxSelectDragEnd.Reset();
    }

    if (state.MouseButtonReleased(SDL_BUTTON_RIGHT))
    {
        if (!CameraDragPos.IsSet())
        {
            // Issue move command to selection
            Command command;
            command.Type = state.KeyDown(SDL_KMOD_SHIFT) ? ECommandType::Queued : ECommandType::Order;
            command.AbilityId = "MoveAbility"_n;
            command.CommandIndex = 0;
            command.TargetLocation = state.GetWorldMousePos();
            Session->EnqueueAction(command);
        }

        CursorDragStart.Reset();
        CameraDragPos.Reset();
    }
}
