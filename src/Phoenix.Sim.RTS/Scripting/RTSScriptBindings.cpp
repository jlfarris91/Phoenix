#include "PhoenixRTS/Scripting/RTSScriptBindings.h"

#include "PhoenixRTS/Orders/FeatureOrders.h"
#include "PhoenixRTS/Units/FeatureUnit.h"

using namespace Phoenix;
using namespace Phoenix::RTS;

// ── Glue functions ────────────────────────────────────────────────────────────
//
// World is injected by the WASM trampoline — not passed from Lua.

// ── Registration ──────────────────────────────────────────────────────────────

void RTSScriptBindings::Describe(ScriptModuleBuilder& builder) const
{
    builder
        .Class<UnitId>("Phoenix.Unit")
        .Inherits("Phoenix.Entity")
        .Method("IsAlive(world, unit)",                             &FeatureUnit::UnitIsAlive)
        .Method("IssueCommand(world, unit, command)",               &FeatureOrders::StaticIssueCommand);
}
