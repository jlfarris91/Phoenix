#pragma once

namespace Phoenix
{
    class Session;
    class World;
    class TypeDescriptor;
    class MethodDescriptor;

    // ── IScriptRuntime ────────────────────────────────────────────────────────
    //
    // VM-agnostic interface for a script runtime (Lua, QuickJS, Wren, …).

    class IScriptRuntime
    {
    public:
        virtual ~IScriptRuntime() = default;

        // ── Declarative registration ──────────────────────────────────────────

        // Registers all methods in a TypeDescriptor into the type's Namespace table.
        // Called once per registered type during session init.
        // Types without a "Namespace" metadata entry are skipped.
        virtual void RegisterType(const TypeDescriptor& desc) = 0;

        // ── Manual registration (used by IScriptBindings) ─────────────────────

        // Opens a dot-separated namespace (e.g. "Phoenix.Unit").
        // Subsequent RegisterFunction calls are placed inside it.
        virtual void OpenNamespace(const char* dotSeparatedPath) = 0;
        virtual void CloseNamespace() = 0;

        // Registers a single function in the currently open namespace.
        virtual void RegisterFunction(const MethodDescriptor& fn) = 0;

        // ── World context ─────────────────────────────────────────────────────

        // Called by FeatureLua before/after each world-scoped Lua callback.
        // Sets the implicit world available to registered world_functions.
        virtual void SetCurrentWorld(World* world) = 0;

        // ── Lifecycle ─────────────────────────────────────────────────────────

        // Called once after all bindings are registered, before any script loads.
        virtual void OnBindingsComplete() = 0;
    };
}
