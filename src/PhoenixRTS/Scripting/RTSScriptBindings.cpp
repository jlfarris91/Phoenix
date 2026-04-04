#include "PhoenixRTS/Scripting/RTSScriptBindings.h"

#include "PhoenixRTS/Orders/FeatureOrders.h"
#include "PhoenixRTS/Units/FeatureUnit.h"

using namespace Phoenix;
using namespace Phoenix::RTS;

// ── Glue functions ────────────────────────────────────────────────────────────
//
// World is injected by the WASM trampoline — not passed from Lua.

static UnitId PhxUnit_Spawn(WorldRef w, FName unitData, uint8 owner, Vec2 pos, Angle facing)
{
    return FeatureUnit::SpawnUnit(w, unitData, owner, pos, facing);
}

// ── Registration ──────────────────────────────────────────────────────────────

void RTSScriptBindings::Describe(ScriptModuleBuilder& builder) const
{
    builder.Class<UnitId>("Phoenix.Unit")
        .Inherits("Phoenix.Entity")
        .Method("IsAlive(world, unit)",                             &FeatureUnit::UnitIsAlive)
        .Method("IssueCommand(world, unit, command)",               &FeatureOrders::StaticIssueCommand)
        .StaticMethod("Spawn(world, unitData, owner, pos, facing)", &PhxUnit_Spawn)
        .End();
}
