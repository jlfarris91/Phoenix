#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "PhoenixScript/DLLExport.h"
#include "PhoenixSim/Reflection/MethodDescriptor.h"
#include "PhoenixSim/Scripting/IScriptRuntime.h"

namespace Phoenix
{
    class WasmRuntime;

    // ── WasmEnvironment ──────────────────────────────────────────────────────
    //
    // Owns one wasm3 IM3Environment + IM3Runtime (linear memory) per world.
    //
    // Architecture:
    //  • FeatureScript creates one WasmEnvironment per world on OnWorldInitialize.
    //  • All worlds share the same raw WASM bytes but get independent runtimes,
    //    so they can be updated in parallel with no shared mutable state.
    //  • World-typed parameters in host functions are detected by TypeDescriptor
    //    and injected automatically — not passed from the WASM side.
    //
    // Rollback:
    //  • Snapshot() memcpy's linear memory into an internal buffer.
    //  • Restore() replays that buffer back into the live runtime.
    //  • Full BlockBuffer integration (for seamless session-level rollback) is TODO.
    //
    // WASM calling convention (script exports):
    //  • No-arg exports:   OnPreUpdate, OnUpdate, OnPostUpdate,
    //                      OnPreWorldUpdate, OnWorldUpdate, OnPostWorldUpdate,
    //                      OnWorldInitialize, OnWorldShutdown
    //  • All are optional — missing exports are silently skipped.

    class PHOENIX_SCRIPT_API WasmEnvironment
    {
    public:
        // Context stored as wasm3 raw-function userdata.
        // Public so the file-local trampoline in .cpp can access members.
        struct CallCtx
        {
            WasmEnvironment* Runtime;
            MethodDescriptor Descriptor;
        };

        WasmEnvironment(
            Session*                           session,
            World*                             world,
            const std::shared_ptr<WasmRuntime>& wasmRuntime);

        ~WasmEnvironment();

        WasmEnvironment(const WasmEnvironment&) = delete;
        WasmEnvironment& operator=(const WasmEnvironment&) = delete;

        bool   IsValid()   const { return ScriptRuntime != nullptr; }
        World* GetWorld()  const { return ScriptWorld; }

        // Call a no-arg WASM export (e.g. "OnWorldUpdate").
        // Returns false if the export does not exist.
        bool CallVoid(const char* name);

        // Load a Lua script from disk into the WASM interpreter.
        // Writes the file bytes into the WASM buffer (via GetScriptBuffer/LoadScript
        // exports) and stores the path for ReloadLuaScript().
        // Returns false on file-read failure or missing WASM exports.
        bool LoadLuaScript(const std::filesystem::path& path);

        // Re-read and reload the last successfully loaded Lua script.
        // The new script takes effect on the next lifecycle callback.
        bool ReloadLuaScript();

        // Execute a Lua code string inside the running Lua state.
        // Writes code into the script buffer and calls the RunString WASM export.
        // The global state (variables, functions) is preserved between calls.
        // Returns false if the WASM export is missing or Lua reports an error.
        bool RunString(const std::string& code);

        // Snapshot / restore linear memory for rollback.
        void Snapshot();
        void Restore();

    private:

        // Look up (and cache) a WASM export by name. Returns nullptr if absent.
        void* FindExport(const char* name);

        // wasm3 opaque handles stored as void* to avoid including wasm3.h here.
        void*    Env     = nullptr;   // IM3Environment
        void*    Runtime = nullptr;   // IM3Runtime
        Session* ScriptSession = nullptr;
        World*   ScriptWorld   = nullptr;

        std::shared_ptr<WasmRuntime> ScriptRuntime;

        // Stable storage for host-function descriptors.
        // CallContexts is reserved to exact capacity before any pointer
        // is passed to wasm3, guaranteeing no reallocation.
        std::vector<CallCtx> CallContexts;

        // IM3Function* cache (stored as void* to avoid the wasm3.h dependency).
        std::unordered_map<std::string, void*> ExportCache;

        // In-process snapshot of WASM linear memory for rollback.
        std::vector<uint8_t> MemorySnapshot;

        // Path of the last successfully loaded Lua script (for ReloadLuaScript).
        std::filesystem::path LuaScriptPath;
    };

} // namespace Phoenix
