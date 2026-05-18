#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <type_traits>
#include <unordered_map>

#include "Phoenix.Sim.Script/DLLExport.h"
#include "Phoenix.Sim.Script/WasmEnvironment.h"
#include "Phoenix.Sim/Features.h"
#include "Phoenix/Name.h"
#include "Phoenix.Sim/Containers/FixedMemory.h"
#include "Phoenix.Sim/Worlds.h"

#ifndef PHX_SCRIPT_WASM_MEMORY_CAPACITY
#define PHX_SCRIPT_WASM_MEMORY_CAPACITY (4 * 1024 * 1024 * 10) // 10x 4 MiB default memory size
#endif

namespace Phoenix
{
    class WasmRuntime;

    struct PHOENIX_SCRIPT_API FeatureScriptDynamicBlock : BlockBufferBlock
    {
        PHX_DECLARE_BLOCK_WITH_ALLOC(FeatureScriptDynamicBlock)
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
        {
            FEATURE_CHANNEL(FeatureChannels::PreUpdate)
            FEATURE_CHANNEL(FeatureChannels::Update)
            FEATURE_CHANNEL(FeatureChannels::PostUpdate)
            FEATURE_CHANNEL(FeatureChannels::WorldInitialize)
            FEATURE_CHANNEL(FeatureChannels::WorldShutdown)
            FEATURE_CHANNEL(FeatureChannels::PreWorldUpdate)
            FEATURE_CHANNEL(FeatureChannels::WorldUpdate)
            FEATURE_CHANNEL(FeatureChannels::PostWorldUpdate)
        }

    public:

        // Called by other features (e.g. FeatureLua) before OnWorldInitialize fires.
        // Loads wasmPath as a shared WasmRuntime and creates a TEnv (defaults to
        // WasmEnvironment) for the given world.  TEnv must derive from WasmEnvironment
        // and share its constructor signature (Session*, World*, shared_ptr<WasmRuntime>).
        // Returns the typed pointer for immediate configuration, or nullptr on failure.
        // FeatureScript::OnWorldInitialize will later call the lifecycle callbacks
        // on this pre-created environment rather than creating a new one.
        template<typename TEnv = WasmEnvironment>
        TEnv* RegisterWorldRuntime(WorldRef world, const std::filesystem::path& wasmPath)
        {
            static_assert(std::is_base_of_v<WasmEnvironment, TEnv>,
                          "TEnv must derive from WasmEnvironment");
            auto runtime = GetOrLoadWasmRuntime(wasmPath);
            if (!runtime)
                return nullptr;
            auto env = std::make_unique<TEnv>(Session.get(), &world, runtime);
            if (!env->IsValid())
                return nullptr;
            auto* ptr = env.get();
            Environments.emplace(world.GetId(), std::move(env));
            return ptr;
        }

        // Returns the environment for a world, or nullptr.
        WasmEnvironment* GetEnvironment(WorldRef world) const;

    protected:

        // ── IFeature lifecycle ────────────────────────────────────────────────

        void Initialize(const std::shared_ptr<Phoenix::Session>& session) override;
        void Shutdown() override;

        void OnPreUpdate(const FeatureUpdateArgs& args) override;
        void OnUpdate(const FeatureUpdateArgs& args) override;
        void OnPostUpdate(const FeatureUpdateArgs& args) override;

        void OnWorldLayout(const WorldLayoutContext& context, BlockBufferConfigBuilder& builder) override;
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
