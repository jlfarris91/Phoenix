#pragma once

#include <string>

#include "PhoenixSim/Reflection/Reflection.h"
#include "PhoenixSim/WorldsFwd.h"

namespace Phoenix
{
    class Session;

    // ── IScriptRuntime ────────────────────────────────────────────────────────
    //
    // VM-agnostic interface for a script runtime (Lua, QuickJS, Wren, …).
    // No sol2 or lua.h headers appear here.
    //
    // Used by:
    //  • FeatureLua, which owns the concrete implementation (LuaRuntime).
    //  • IScriptBindings, for manual function registration.

    class IScriptRuntime
    {
    public:
        virtual ~IScriptRuntime() = default;

        // ── Declarative registration ──────────────────────────────────────────

        // Registers all script functions in a TypeDescriptor under its ScriptNamespace.
        // Called once per type during session init. Types with empty ScriptNamespace
        // are skipped.
        virtual void RegisterType(const TypeDescriptor& desc) = 0;

        // ── Manual registration (used by IScriptBindings) ─────────────────────

        // Opens a dot-separated namespace (e.g. "Phoenix.Unit").
        // Subsequent RegisterFunction calls are placed inside it.
        virtual void OpenNamespace(const char* dotSeparatedPath) = 0;
        virtual void CloseNamespace() = 0;

        // Registers a single function in the currently open namespace.
        virtual void RegisterFunction(const MethodDescriptor& fn) = 0;

        // ── Script loading ────────────────────────────────────────────────────

        virtual bool LoadFile(const char* path) = 0;
        virtual bool ExecString(const std::string& code) = 0;

        // ── World context ─────────────────────────────────────────────────────

        // Called by FeatureLua before/after each world-scoped Lua callback.
        // Sets the implicit world available to registered world_functions.
        virtual void SetCurrentWorld(World* world) = 0;

        // ── Lifecycle ─────────────────────────────────────────────────────────

        // Called once after all bindings are registered, before any script loads.
        virtual void OnBindingsComplete() = 0;
    };
}
