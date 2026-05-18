#pragma once

#include "PhoenixSim/Reflection/Registration.h"
#include "PhoenixSim/Scripting/IScriptBindings.h"

namespace Phoenix::RTS
{
    // ── RTSScriptBindings ─────────────────────────────────────────────────────
    //
    // Script bindings for PhoenixRTS.
    //
    // Exposes a Unit class in Lua with method-call syntax:
    //
    //   unit:IsAlive()                     → bool
    //   unit:IssueCommand(command)         → bool
    //
    // And a static constructor:
    //
    //   Phoenix.Unit.Spawn(unitData, owner, pos, facing) → UnitId
    //
    // IssueCommand is intentionally placed on Unit rather than Orders — it is
    // a cleaner API for scripts. Phoenix.Orders.IssueCommand still exists via
    // reflection for parity.

    class RTSScriptBindings : public IScriptBindings
    {
        PHX_DECLARE_TYPE_DERIVED(RTSScriptBindings, IScriptBindings)

    public:
        void Describe(ScriptModuleBuilder& builder) const override;
    };
}
