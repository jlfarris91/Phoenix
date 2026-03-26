#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "PhoenixScript/DLLExport.h"
#include "PhoenixScript/WasmEnvironment.h"
#include "PhoenixSim/Features.h"
#include "PhoenixSim/Name.h"
#include "PhoenixSim/Containers/FixedMemory.h"

#ifndef PHX_SCRIPT_WASM_MEMORY_CAPACITY
#define PHX_SCRIPT_WASM_MEMORY_CAPACITY (4 * 1024 * 1024 * 10) // 10x 4 MiB default memory size
#endif

namespace Phoenix
{
    class WasmRuntime;

    struct PHOENIX_SCRIPT_API FeatureScriptDynamicBlock : public BufferBlockBase
    {
        PHX_DECLARE_BLOCK_WITH_ALLOC(FeatureScriptDynamicBlock)

        struct Config
        {
            uint32 WasmMemoryCapacity = PHX_SCRIPT_WASM_MEMORY_CAPACITY;
        };

        TFixedStorage<uint8> WasmMemory;
    };

    // ── FeatureScript ─────────────────────────────────────────────────────────
    //
    // Replaces FeatureLua with a WASM-based scripting runtime.
    //
    // One WasmWorldRuntime is created per world on OnWorldInitialize, giving
    // each world an independent wasm3 IM3Runtime (linear memory).  Worlds can
    // therefore be stepped in parallel without any shared scripting state.
    //
    // Host imports (C++ functions callable from WASM) are collected during
    // Initialize by iterating the TypeRegistry and any IScriptBindings services.
    // The collected registrations are then linked into each world's runtime when
    // it is created.
    //
    // Script callbacks (WASM exports):
    //   OnPreUpdate / OnUpdate / OnPostUpdate
    //   OnPreWorldUpdate / OnWorldUpdate / OnPostWorldUpdate
    //   OnWorldInitialize / OnWorldShutdown
    // All are optional — if an export is absent it is silently skipped.

    class PHOENIX_SCRIPT_API FeatureScript : public IFeature
    {
        PHX_DECLARE_FEATURE_TYPE(FeatureScript)

    public:

        FeatureScript();

        // Called by other features (e.g. FeatureLua) before OnWorldInitialize fires.
        // Loads wasmPath as a shared WasmRuntime and creates a WasmEnvironment for
        // the given world.  Returns the environment so the caller can configure it
        // (e.g. call LoadLuaScript).  Returns nullptr on load failure.
        // FeatureScript::OnWorldInitialize will later call the lifecycle callbacks
        // on this pre-created environment rather than creating a new one.
        WasmEnvironment* RegisterWorldRuntime(WorldRef world, const std::filesystem::path& wasmPath);

        // Returns the environment for a world, or nullptr.
        WasmEnvironment* GetEnvironment(WorldRef world) const;

    protected:

        // ── IFeature lifecycle ────────────────────────────────────────────────

        void Initialize(const std::shared_ptr<Phoenix::Session>& session) override;
        void Shutdown() override;

        void OnPreUpdate(const FeatureUpdateArgs& args) override;
        void OnUpdate(const FeatureUpdateArgs& args) override;
        void OnPostUpdate(const FeatureUpdateArgs& args) override;

        void OnWorldLayout(const WorldLayoutContext& context, BlockBufferLayoutBuilder& builder) override;
        void OnWorldInitialize(WorldRef world) override;
        void OnWorldShutdown(WorldRef world) override;

        void OnPreWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;
        void OnWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;
        void OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;

    private:

        std::shared_ptr<WasmRuntime> GetRuntime(const std::string& fileName) const;
        std::shared_ptr<WasmRuntime> GetOrLoadWasmRuntime(const std::filesystem::path& path);

        std::unordered_map<std::string, std::shared_ptr<WasmRuntime>> Runtimes;

        // One runtime per world, keyed by world FName hash.
        std::unordered_map<hash32_t, std::unique_ptr<WasmEnvironment>> Environments;
    };

} // namespace Phoenix
