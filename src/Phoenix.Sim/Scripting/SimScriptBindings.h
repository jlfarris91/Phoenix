#pragma once

#include "Phoenix/Reflection/Registration.h"
#include "Phoenix.Sim/Scripting/IScriptBindings.h"

namespace Phoenix
{
    // ── SimScriptBindings ─────────────────────────────────────────────────────
    //
    // Script bindings for PhoenixSim that cannot be expressed via reflection:
    //
    //   Phoenix.GetWorldId()         → FName  (hash of the world's id)
    //   Phoenix.GetWorldType()       → FName  (hash of the world's type)
    //   Phoenix.RandomRange(min,max) → int32  (deterministic world RNG)
    //   Phoenix.RandomFloat()        → float  (deterministic world RNG, [0,1))
    //
    // World is injected automatically by the WASM trampoline — not a Lua arg.
    // These are pure glue; the underlying World member functions are not
    // registered in the reflection system.

    class SimScriptBindings : public IScriptBindings
    {
        PHX_DECLARE_TYPE_DERIVED(SimScriptBindings, IScriptBindings)

    public:
        void Describe(ScriptModuleBuilder& builder) const override;
    };
}
