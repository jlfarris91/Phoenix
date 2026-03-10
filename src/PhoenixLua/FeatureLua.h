
#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "PhoenixLua/DLLExport.h"
#include "PhoenixLua/LuaFP64.h"
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
    };
}
