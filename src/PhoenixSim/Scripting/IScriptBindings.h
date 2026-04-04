#pragma once

#include "PhoenixSim/Reflection/Registration.h"
#include "PhoenixSim/Scripting/ScriptModuleBuilder.h"

namespace Phoenix
{
    // ── IScriptBindings ───────────────────────────────────────────────────────
    //
    // Declarative DDL interface for script bindings that cannot (or should not)
    // be expressed via PHX_DEFINE_TYPE reflection.
    //
    // Examples:
    //  • Namespace remapping (IssueCommand under Phoenix.Unit instead of Phoenix.Orders)
    //  • OOP wrappers (UnitId metatable with :IsAlive(), :IssueCommand())
    //  • Glue functions with no C++ equivalent (GetWorldId, GetWorldType)
    //
    // Implementations are discovered at build time by PhoenixWasmGen and
    // PhoenixLuaGen (and at runtime by WasmRuntime) via:
    //
    //   TypeRegistry::GetAllDerivedFrom<IScriptBindings>()
    //
    // Each discovered concrete type is default-constructed, Describe() is
    // called, then the instance is destroyed.  Implementations must be
    // default-constructible (guaranteed automatically by TypeRegistry::Get<T>
    // for any default-constructible type).
    //
    // WasmGen flattens all Class methods to static host imports.
    // LuaGen composes Class entries into metatables and Namespace entries
    // into Lua table hierarchies.

    class IScriptBindings
    {
        PHX_DECLARE_TYPE_INTERFACE(IScriptBindings)

    public:
        virtual ~IScriptBindings() = default;

        // Populate builder with this module's script API declarations.
        virtual void Describe(ScriptModuleBuilder& builder) const = 0;
    };

} // namespace Phoenix
