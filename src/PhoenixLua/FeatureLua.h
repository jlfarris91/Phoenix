
#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "PhoenixLua/DLLExport.h"
#include "PhoenixLua/LuaFP64.h"
#include "PhoenixLua/LuaRuntime.h"
#include "PhoenixSim/Features.h"

namespace Phoenix
{
    struct PHOENIX_LUA_API FeatureLuaDynamicBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK(FeatureLuaDynamicBlock)

        sol::state State;
    };

    class PHOENIX_LUA_API FeatureLua : public IFeature
    {
        PHX_DECLARE_FEATURE_TYPE(FeatureLua)

    public:

        FeatureLua();

        // Sets the path of the script loaded on Initialize (call before Initialize).
        void SetScriptPath(std::string path);

        // Thread-safe: enqueues a Lua code string to be executed on the next sim tick.
        void EnqueueScript(std::string code);

        void Initialize(const std::shared_ptr<Phoenix::Session>& session) override;
        void Shutdown() override;

        void OnPreUpdate(const FeatureUpdateArgs& args) override;
        void OnUpdate(const FeatureUpdateArgs& args) override;
        void OnPostUpdate(const FeatureUpdateArgs& args) override;

        bool OnPreHandleAction(const FeatureActionArgs& action) override;
        bool OnHandleAction(const FeatureActionArgs& action) override;
        bool OnPostHandleAction(const FeatureActionArgs& action) override;

        void OnWorldInitialize(WorldRef world) override;
        void OnWorldShutdown(WorldRef world) override;

        void OnPreWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;
        void OnWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;
        void OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;

        bool OnPreHandleWorldAction(WorldRef world, const FeatureActionArgs& action) override;
        bool OnHandleWorldAction(WorldRef world, const FeatureActionArgs& action) override;
        bool OnPostHandleWorldAction(WorldRef world, const FeatureActionArgs& action) override;

    private:
        std::unique_ptr<LuaRuntime> m_luaRuntime;

        std::string              m_scriptPath;
        std::mutex               m_scriptQueueMutex;
        std::vector<std::string> m_scriptQueue;
    };
}
