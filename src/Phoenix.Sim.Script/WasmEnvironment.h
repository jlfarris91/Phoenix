#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Phoenix.Sim.Script/DLLExport.h"
#include "Phoenix/Reflection/MethodDescriptor.h"
#include "Phoenix.Sim/Scripting/IScriptRuntime.h"

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
    //  • Update exports accept a float dt parameter:
    //      OnPreWorldUpdate(dt), OnWorldUpdate(dt), OnPostWorldUpdate(dt)
    //  • No-arg exports:   OnPreUpdate, OnUpdate, OnPostUpdate,
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

        virtual ~WasmEnvironment();

        WasmEnvironment(const WasmEnvironment&) = delete;
        WasmEnvironment& operator=(const WasmEnvironment&) = delete;

        bool   IsValid()   const { return ScriptRuntime != nullptr; }
        World* GetWorld()  const { return ScriptWorld; }

        // Call a named WASM export with arguments and retrieve results.
        // argPtrs: array of pointers to argument values (nullptr if argc == 0).
        // retPtrs: array of pointers where results are written (nullptr if retc == 0).
        // Mirrors the m3_Call / m3_GetResults convention used by wasm3.
        bool CallExport(const char* name, int argc, const void** argPtrs, int retc, const void** retPtrs);

        // Returns a pointer to the WASM linear memory and optionally its size in bytes.
        // Returns nullptr if the runtime is not valid.
        uint8_t* GetMemory(uint32_t* outSize = nullptr) const;

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
    };

} // namespace Phoenix
