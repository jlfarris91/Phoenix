
#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "PhoenixLua/DLLExport.h"
#include "PhoenixSim/Features.h"

namespace Phoenix
{
    // ── FeatureLua ────────────────────────────────────────────────────────────
    //
    // Lua scripting feature built on PhoenixScript's WASM infrastructure.
    //
    // Architecture:
    //  • Delegates WASM runtime management to FeatureScript.
    //  • On OnWorldInitialize: calls FeatureScript::RegisterWorldRuntime with
    //    the shared lua.wasm binary, then calls LoadLuaScript with the
    //    world's configured .lua file.
    //  • FeatureScript drives all lifecycle callbacks (OnPreUpdate,
    //    OnWorldUpdate, etc.) — FeatureLua only orchestrates the Lua layer.
    //  • EnqueueScript allows thread-safe submission of Lua snippets; they
    //    are executed on the next OnWorldUpdate tick via WasmEnvironment::RunString.
    //
    // World config (under "FeatureLua"):
    //   "script": "path/to/script.lua"   (relative to worlds directory)
    //
    // lua.wasm is loaded from <DataDirectory>/lua.wasm.

    class PHOENIX_LUA_API FeatureLua : public IFeature
    {
        PHX_DECLARE_FEATURE_TYPE(FeatureLua)
        {
            FEATURE_CHANNEL(FeatureChannels::WorldInitialize)
            FEATURE_CHANNEL(FeatureChannels::WorldShutdown)
            FEATURE_CHANNEL(FeatureChannels::WorldUpdate)
        }

    public:

        // Thread-safe: enqueues a Lua snippet to execute on the next world update.
        void EnqueueScript(std::string code);

        void Initialize(const std::shared_ptr<Phoenix::Session>& session) override;
        void Shutdown() override;

        void OnWorldInitialize(WorldRef world) override;
        void OnWorldShutdown(WorldRef world) override;
        void OnWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;

    private:
        std::mutex               ScriptQueueMutex;
        std::vector<std::string> ScriptQueue;
    };
}
