#include "PhoenixSim/Scripting/SimScriptBindings.h"

#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/Worlds.h"

using namespace Phoenix;
using namespace Phoenix::ECS;

// ── World glue functions ──────────────────────────────────────────────────────
//
// World is injected by the WASM trampoline — not passed from Lua.

static FName  PhxSim_GetWorldId   (WorldConstRef w) { return w.GetId(); }
static FName  PhxSim_GetWorldType (WorldConstRef w) { return w.GetType(); }
static int32  PhxSim_RandomRange  (WorldRef w, int32 min, int32 max) { return w.GetRandom().RandomRange32(min, max); }
static float  PhxSim_RandomFloat  (WorldRef w) { return static_cast<float>(w.GetRandom().NextU32()) / 4294967296.0f; }

// ── Registration ──────────────────────────────────────────────────────────────

PHX_DEFINE_TYPE(Phoenix::SimScriptBindings)
{
    registration.Bases<IScriptBindings>();
}

void SimScriptBindings::Describe(ScriptModuleBuilder& builder) const
{
    builder.Namespace("Phoenix")
        .Function("GetWorldId(world)",              &PhxSim_GetWorldId)
        .Function("GetWorldType(world)",            &PhxSim_GetWorldType)
        .Function("RandomRange(world, min, max)",   &PhxSim_RandomRange)
        .Function("RandomFloat(world)",             &PhxSim_RandomFloat)
        .End();

    builder.Class<EntityId>("Phoenix.Entity")
        .Method("IsValid(world, entity)",           &FeatureECS::IsEntityValid)
        .Method("HasTag(world, entity, tag)",       &FeatureECS::HasTag)
        .Method("AddTag(world, entity, tag)",       &FeatureECS::AddTag)
        .Method("RemoveTag(world, entity, tag)",    &FeatureECS::RemoveTag)
        .Method("GetPosition(world, entity)",       &FeatureECS::GetWorldPosition)
        .Method("GetFacing(world, entity)",         &FeatureECS::GetWorldFacing)
        .End();
}
