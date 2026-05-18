#include "Phoenix.Sim.Steering/ScriptBindings.h"

#include "FeatureSteering.h"
#include "Phoenix.Sim.ECS/EntityId.h"

using namespace Phoenix;
using namespace Phoenix::Steering;

// ── Glue functions ────────────────────────────────────────────────────────────
//
// World is injected by the WASM trampoline — not passed from Lua.



// ── Registration ──────────────────────────────────────────────────────────────

void ScriptBindings::Describe(ScriptModuleBuilder& builder) const
{
    builder
        .Namespace("Phoenix.Steering")
        .Function("MoveToLocation(world, entity, target, range)", &FeatureSteering::MoveToLocation)
        .Function("FollowEntity(world, entity, target, range)",   &FeatureSteering::FollowEntity)
        .Function("IsMoving(world, entity)",                      &FeatureSteering::IsMoving)
        .Function("HasFinishedMoving(world, entity)",             &FeatureSteering::HasFinishedMoving)
        .Function("TurnToFaceEntity(world, entity, target)",      static_cast<bool(*)(WorldRef, const ECS::EntityId&, const ECS::EntityId&)>(&FeatureSteering::TurnToFace))
        .Function("TurnToFacePos(world, entity, target)",         static_cast<bool(*)(WorldRef, const ECS::EntityId&, const ECS::EntityId&)>(&FeatureSteering::TurnToFace))
        .Function("IsTurning(world, entity)",                     &FeatureSteering::IsTurning)
        .Function("HasFinishedTurning(world, entity)",            &FeatureSteering::HasFinishedTurning)
        .Function("GetSteeringMode(world, entity)",               &FeatureSteering::GetSteeringMode)
        .Function("IsSeekingGoal(world, entity)",                 &FeatureSteering::IsSeekingGoal)
        .Function("HasArrivedAtGoal(world, entity)",              &FeatureSteering::HasArrivedAtGoal)
        .Function("Stop(world, entity)",                          &FeatureSteering::Stop)
        .Function("IsHolding(world, entity)",                     &FeatureSteering::IsHolding)
        .Function("SetHolding(world, entity, holding)",           &FeatureSteering::SetHolding)
        .Function("UpdateSpeed(world, entity, args)",             &FeatureSteering::UpdateSpeed)
        .Function("GetInnerRadius(world, entity)",                &FeatureSteering::GetEntityInnerRadius)
        .Function("GetOuterRadius(world, entity)",                &FeatureSteering::GetEntityOuterRadius)
        .End();
}
