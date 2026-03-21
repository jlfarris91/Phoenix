#include "PlayerController.h"
#include <PhoenixSim/Reflection/TypeRegistrationBuilder.h>

#include <SDL3/SDL_events.h>

#include <PhoenixSim/ECS/FeatureECS.h>
#include <PhoenixSim/FixedPoint/FixedVector.h>
#include <PhoenixSim/Flags.h>
#include <PhoenixSim/Session.h>

#include <PhoenixRTS/Orders/Commands.h>
#include <PhoenixRTS/Selection/FeatureSelection.h>
#include <PhoenixRTS/Orders/FeatureOrders.h>
#include <PhoenixRTS/Units/UnitId.h>

#include "../SDL/SDLCamera.h"
#include "../SDL/SDLDebugState.h"
#include "../SDL/SDLDebugRenderer.h"
#include "../SDL/SDLUtils.h"
#include "PhoenixRTS/Data/DataUnit.h"
#include "PhoenixRTS/Units/FeatureUnit.h"
#include "PhoenixSim/LDS/FeatureLDS.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

PlayerController::PlayerController(const std::shared_ptr<Phoenix::Session>& session, SDLCamera* camera, SDLViewport* viewport)
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

    const LDS::ILDSQueryContext& lds = *LDS::FeatureLDS::StaticGetWorldQueryContext(world);

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

        std::vector<EntityTransform> entities;
        FeatureECS::QueryEntitiesInRect(world, min, max, entities);

        for (const EntityTransform& entity : entities)
        {
            DrawSelectionCircle(world, renderer, entity.EntityId);
        }
    }
    else if (FeatureUnit::UnitIsAlive(world, UnitId(HoverTarget)))
    {
        DrawSelectionCircle(world, renderer, HoverTarget);
    }

    EntityId selectionGroup = FeatureSelection::GetPlayerSelection(world, playerId, groupId);
    FeatureECS::ForEachEntityInGroup(world, selectionGroup, [&](const EntityId& entityId)
    {
        auto transformPtr = FeatureECS::GetWorldTransformPtr(world, entityId);
        if (!transformPtr)
        {
            return;
        }

        DrawSelectionCircle(world, renderer, entityId);

        Vec2 lastTargetPos = transformPtr->Position;
        FeatureOrders::ForEachOrder(world, UnitId(entityId), [&](const Order& order)
        {
            Vec2 orderPos = order.TargetLocation;

            if (order.TargetEntity != EntityId::Invalid && FeatureECS::IsEntityValid(world, order.TargetEntity))
            {
                orderPos = FeatureECS::GetWorldPosition(world, order.TargetEntity);
            }

            renderer.DrawLine(lastTargetPos, orderPos, Color::Blue);
            lastTargetPos = order.TargetLocation;
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

            std::vector<EntityTransform> entities;
            FeatureECS::QueryEntitiesInRect(world, min, max, entities);

            if (!state.KeyDown(SDL_KMOD_CTRL) && !state.KeyDown(SDL_KMOD_SHIFT))
            {
                Action clearAction;
                clearAction.Verb = "player_selection_clear"_n;
                clearAction.Args[0].AsUInt32 = playerId;
                clearAction.Args[1].AsUInt32 = (uint32)groupId;
                Session->EnqueueAction(clearAction);
            }

            for (const EntityTransform& entity : entities)
            {
                FName verb = state.KeyDown(SDL_KMOD_CTRL)
                    ? "player_selection_remove"_n
                    : "player_selection_add"_n;

                Action clearAction;
                clearAction.Verb = verb;
                clearAction.Args[0].AsUInt32 = playerId;
                clearAction.Args[1].AsUInt32 = (uint32)groupId;
                clearAction.Args[2].AsUInt32 = entity.EntityId;
                Session->EnqueueAction(clearAction);
            }
        }
        else
        {
            std::vector<EntityTransform> entities;
            FeatureECS::QueryEntitiesInRange(world, CursorWorldPos, 1.0, entities);

            if (!state.KeyDown(SDL_KMOD_CTRL) && !state.KeyDown(SDL_KMOD_SHIFT))
            {
                Action clearAction;
                clearAction.Verb = "player_selection_clear"_n;
                clearAction.Args[0].AsUInt32 = playerId;
                clearAction.Args[1].AsUInt32 = (uint32)groupId;
                Session->EnqueueAction(clearAction);
            }

            for (const EntityTransform& entity : entities)
            {
                FName verb = state.KeyDown(SDL_KMOD_CTRL)
                    ? "player_selection_remove"_n
                    : "player_selection_add"_n;

                Action clearAction;
                clearAction.Verb = verb;
                clearAction.Args[0].AsUInt32 = playerId;
                clearAction.Args[1].AsUInt32 = (uint32)groupId;
                clearAction.Args[2].AsUInt32 = entity.EntityId;
                Session->EnqueueAction(clearAction);
                break;
            }
        }

        CursorDragStart.Reset();
        BoxSelectDragStart.Reset();
        BoxSelectDragEnd.Reset();
    }

    {
        std::vector<EntityTransform> entities;
        FeatureECS::QueryEntitiesInRange(world, CursorWorldPos, 1.0, entities);

        EntityId closestEntity;
        Distance closestDist = Distance::Max;
        for (const EntityTransform& entity : entities)
        {
            if (FeatureUnit::UnitIsDead(world, UnitId(entity.EntityId)))
            {
                continue;
            }

            Distance dist = Vec2::Distance(entity.TransformComponent->Transform.Position, CursorWorldPos);
            if (dist < closestDist)
            {
                closestDist = dist;
                closestEntity = entity.EntityId;
            }

            break;
        }

        HoverTarget = closestEntity;
    }

    if (state.MouseButtonReleased(SDL_BUTTON_RIGHT))
    {
        if (!CameraDragPos.IsSet())
        {
            FName abilityId = "MoveAbility"_n;
            uint8 commandIndex = 0;
            ECommandFlags commandFlags = state.KeyDown(SDL_KMOD_SHIFT) ? ECommandFlags::Queue : ECommandFlags::Replace;
            EntityId targetEntity = EntityId::Invalid;
            Vec2 targetLocation = state.GetWorldMousePos();

            if (HoverTarget != EntityId::Invalid)
            {
                SetFlagRef(commandFlags, ECommandFlags::Smart);
                abilityId = FName::None;
                commandIndex = 0;
                targetEntity = HoverTarget;
                targetLocation = FeatureECS::GetWorldPosition(world, targetEntity);

                abilityId = "AttackAbility"_n;
            }

            // Issue command to selection
            Command command;
            command.Flags = commandFlags;
            command.CommandId = abilityId;
            command.CommandIndex = commandIndex;
            command.TargetEntity = targetEntity;
            command.TargetLocation = targetLocation;
            Session->EnqueueAction(command);
        }

        CursorDragStart.Reset();
        CameraDragPos.Reset();
    }
}

// ── Type registration ──────────────────────────────────────────────────────────

using namespace Phoenix;

PHX_TYPE_REGISTRATION(PlayerController)
{
    registration
        .Field("PanSpeed",  &PlayerController::PanSpeed)
        .Field("ZoomSpeed", &PlayerController::ZoomSpeed);
}
