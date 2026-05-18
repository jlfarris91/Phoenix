
#pragma once

#include <filesystem>
#include <memory>
#include <string>

#include "Phoenix.Sim.Lua/DLLExport.h"
#include "Phoenix.Sim.Script/WasmEnvironment.h"

namespace Phoenix
{
    class WasmRuntime;

    // ── LuaWasmEnvironment ────────────────────────────────────────────────────
    //
    // Lua-specific WasmEnvironment subclass.
    //
    // WasmEnvironment is language-agnostic — it knows nothing about Lua, scripts,
    // or the lua.wasm export protocol. This subclass adds the Lua layer on top
    // using the inherited CallExport / GetMemory primitives.
    //
    // Created by FeatureLua via FeatureScript::RegisterWorldRuntime<LuaWasmEnvironment>
    // and owned by FeatureScript. FeatureLua holds a typed pointer for Lua-specific
    // operations; all generic lifecycle callbacks are dispatched by FeatureScript.

    class PHOENIX_LUA_API LuaWasmEnvironment : public WasmEnvironment
    {
    public:
        LuaWasmEnvironment(Session* session, World* world,
                           const std::shared_ptr<WasmRuntime>& runtime);

        // Load a Lua script from disk into the WASM interpreter.
        // Writes the file bytes into the WASM script buffer (via GetScriptBuffer /
        // LoadScript exports) and stores the path for ReloadLuaScript().
        // Returns false on file-read failure or missing WASM exports.
        bool LoadLuaScript(const std::filesystem::path& path);

        // Re-read and reload the last successfully loaded Lua script.
        // The new script takes effect on the next lifecycle callback.
        bool ReloadLuaScript();

        // Execute a Lua snippet inside the running Lua state.
        // Writes code into the script buffer and calls the RunString WASM export.
        // The global state (variables, functions) is preserved between calls.
        // Returns false if the WASM export is missing or Lua reports an error.
        bool RunString(const std::string& code);

    private:
        std::filesystem::path LuaScriptPath;
    };

} // namespace Phoenix
