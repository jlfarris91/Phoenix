
#include "PhoenixLua/FeatureLua.h"
#include "PhoenixLua/LuaRuntime.h"
#include "PhoenixSim/Platform.h"

#include <iostream>
#include <lua.h>

#include "PhoenixSim/Flags.h"
#include "PhoenixSim/Session.h"
#include "PhoenixSim/Containers/Optional.h"
#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/Scripting/IScriptBindings.h"
#include "PhoenixSim/Reflection/TypeRegistry.h"

#include "PhoenixLua/LuaFP64.h"

using namespace Phoenix;

FeatureLua::FeatureLua()
{
    FEATURE_SESSION_BLOCK(FeatureLuaDynamicBlock, EBufferBlockType::Dynamic)
    FEATURE_CHANNEL(FeatureChannels::PreUpdate)
    FEATURE_CHANNEL(FeatureChannels::Update)
    FEATURE_CHANNEL(FeatureChannels::PostUpdate)
    FEATURE_CHANNEL(FeatureChannels::PreHandleAction)
    FEATURE_CHANNEL(FeatureChannels::HandleAction)
    FEATURE_CHANNEL(FeatureChannels::PostHandleAction)
    FEATURE_CHANNEL(FeatureChannels::WorldInitialize)
    FEATURE_CHANNEL(FeatureChannels::WorldShutdown)
    FEATURE_CHANNEL(FeatureChannels::PreWorldUpdate)
    FEATURE_CHANNEL(FeatureChannels::WorldUpdate)
    FEATURE_CHANNEL(FeatureChannels::PostWorldUpdate)
    FEATURE_CHANNEL(FeatureChannels::PreHandleWorldAction)
    FEATURE_CHANNEL(FeatureChannels::HandleWorldAction)
    FEATURE_CHANNEL(FeatureChannels::PostHandleWorldAction)
    FEATURE_CHANNEL(FeatureChannels::DebugRender)
}

void FeatureLua::SetScriptPath(std::string path)
{
    m_scriptPath = std::move(path);
}

void FeatureLua::EnqueueScript(std::string code)
{
    std::lock_guard lock(m_scriptQueueMutex);
    m_scriptQueue.push_back(std::move(code));
}

void FeatureLua::Initialize(const std::shared_ptr<Phoenix::Session>& session)
{
    IFeature::Initialize(session);

    BlockBuffer& buffer = Session->GetBuffer();
    FeatureLuaDynamicBlock* dynamicBlock = buffer.GetBlock<FeatureLuaDynamicBlock>();
    if (!dynamicBlock)
        return;

    sol::state& state = dynamicBlock->State;

    state.set_exception_handler([](lua_State*, sol::optional<const std::exception&> ex, sol::string_view v)
    {
        if (ex.has_value())
            std::cout << "Lua error caught in C++: " << ex->what() << "\n";
        else
            std::cout << "Lua error caught in C++: " << v << "\n";
        return 0;
    });

    state.open_libraries(sol::lib::base);
    Lua::LuaOpen_FP64(state);

    // Construct LuaRuntime
    m_luaRuntime = std::make_unique<LuaRuntime>(state, Session.get());

    // ── 1. Declarative registrations from TypeRegistry ───────────────────────

    for (const auto& [_, desc] : TypeRegistry::GetAll())
        m_luaRuntime->RegisterType(*desc);

    // ── 2. Manual IScriptBindings services ───────────────────────────────────

    std::vector<std::shared_ptr<IService>> bindingServices;
    Session->GetServices(IScriptBindings::StaticTypeName, bindingServices);
    for (const auto& svc : bindingServices)
    {
        auto* bindings = static_cast<IScriptBindings*>(svc.get());
        m_luaRuntime->OpenNamespace(bindings->GetNamespace());
        bindings->Register(*m_luaRuntime);
        m_luaRuntime->CloseNamespace();
    }

    // ── 3. Built-ins owned by FeatureLua ─────────────────────────────────────

    // hash("name") → FName integer
    state["hash"] = [](const std::string& v) -> uint32_t
    {
        return static_cast<uint32_t>(static_cast<hash32_t>(FName(v.c_str())));
    };

    auto phoenix = state["Phoenix"].get_or_create<sol::table>();

    // Phoenix.GetSimTime()
    phoenix["GetSimTime"] = [this]() -> double
    {
        return static_cast<double>(Session->GetSimTime());
    };

    // Phoenix.ECS — world is implicit via m_luaRuntime->GetCurrentWorld()
    auto ecs = phoenix["ECS"].get_or_create<sol::table>();

    ecs["AcquireEntity"] = [this](const FName& kind) -> ECS::EntityId
    {
        World* w = m_luaRuntime->GetCurrentWorld();
        if (!w) return ECS::EntityId::Invalid;
        return ECS::FeatureECS::StaticAcquireEntity(*w, kind);
    };

    ecs["ReleaseEntity"] = [this](ECS::EntityId entityId) -> bool
    {
        World* w = m_luaRuntime->GetCurrentWorld();
        if (!w) return false;
        return ECS::FeatureECS::StaticReleaseEntity(*w, entityId);
    };

    // ── 4. Finish and load script ─────────────────────────────────────────────

    m_luaRuntime->OnBindingsComplete();

    if (!m_scriptPath.empty())
        m_luaRuntime->LoadFile(m_scriptPath.c_str());
}

void FeatureLua::Shutdown()
{
    m_luaRuntime.reset();
    IFeature::Shutdown();
}

// ── Lua callback helpers ───────────────────────────────────────────────────────

namespace Phoenix::detail
{
    template <class TRet, class ...TArgs>
    TOptional<TRet> TryExecuteLuaFunctionRet(sol::state& state, const char* funcName, TArgs&& ...args)
    {
        sol::protected_function luaFunc = state[funcName];

        if (!luaFunc.valid())
        {
            std::cout << "Could not find global function named '" << funcName << "'.\n";
            return TOptional<TRet>();
        }

        sol::protected_function_result result = luaFunc(std::forward<TArgs>(args)...);

        if (!result.valid())
        {
            sol::error err = result;
            std::cout << "Lua error caught in C++: " << err.what() << "\n";
            return TOptional<TRet>();
        }

        if (result.return_count() == 0)
            return TOptional<TRet>();

        TRet returnValue = result;

#if 1
        std::cout << "Function '" << funcName << "' returned: " << returnValue << "\n";
#endif

        return returnValue;
    }

    template <class ...TArgs>
    bool TryExecuteLuaFunction(sol::state& state, const char* funcName, TArgs&& ...args)
    {
        sol::protected_function luaFunc = state[funcName];

        if (!luaFunc.valid())
        {
            std::cout << "Could not find global function named '" << funcName << "'.\n";
            return false;
        }

        sol::protected_function_result result = luaFunc(std::forward<TArgs>(args)...);

        if (!result.valid())
        {
            sol::error err = result;
            std::cout << "Lua error caught in C++: " << err.what() << "\n";
            return false;
        }

        return true;
    }

    template <class TRet, class ...TArgs>
    TOptional<TRet> TryExecuteLuaFunctionRet(const std::shared_ptr<Session>& session, const char* funcName, TArgs&& ...args)
    {
        BlockBuffer& buffer = session->GetBuffer();
        FeatureLuaDynamicBlock* dynamicBlock = buffer.GetBlock<FeatureLuaDynamicBlock>();
        if (!dynamicBlock)
            return TOptional<TRet>();

        return TryExecuteLuaFunctionRet<TRet, TArgs...>(dynamicBlock->State, funcName, std::forward<TArgs>(args)...);
    }

    template <class ...TArgs>
    bool TryExecuteLuaFunction(const std::shared_ptr<Session>& session, const char* funcName, TArgs&& ...args)
    {
        BlockBuffer& buffer = session->GetBuffer();
        FeatureLuaDynamicBlock* dynamicBlock = buffer.GetBlock<FeatureLuaDynamicBlock>();
        if (!dynamicBlock)
            return false;

        return TryExecuteLuaFunction(dynamicBlock->State, funcName, std::forward<TArgs>(args)...);
    }
}

// ── Session-level callbacks ───────────────────────────────────────────────────

void FeatureLua::OnPreUpdate(const FeatureUpdateArgs& args)
{
    IFeature::OnPreUpdate(args);
    detail::TryExecuteLuaFunction(Session, "OnPreUpdate");
}

void FeatureLua::OnUpdate(const FeatureUpdateArgs& args)
{
    IFeature::OnUpdate(args);
    detail::TryExecuteLuaFunction(Session, "OnUpdate");
}

void FeatureLua::OnPostUpdate(const FeatureUpdateArgs& args)
{
    IFeature::OnPostUpdate(args);
    detail::TryExecuteLuaFunction(Session, "OnPostUpdate");
}

bool FeatureLua::OnPreHandleAction(const FeatureActionArgs& action)
{
    IFeature::OnPreHandleAction(action);
    TOptional<bool> result = detail::TryExecuteLuaFunctionRet<bool>(Session, "OnPreHandleAction", action.Action);
    return result.GetValue(false);
}

bool FeatureLua::OnHandleAction(const FeatureActionArgs& action)
{
    IFeature::OnHandleAction(action);
    TOptional<bool> result = detail::TryExecuteLuaFunctionRet<bool>(Session, "OnHandleAction", action.Action);
    return result.GetValue(false);
}

bool FeatureLua::OnPostHandleAction(const FeatureActionArgs& action)
{
    IFeature::OnPostHandleAction(action);
    TOptional<bool> result = detail::TryExecuteLuaFunctionRet<bool>(Session, "OnPostHandleAction", action.Action);
    return result.GetValue(false);
}

// ── World-scoped callbacks ────────────────────────────────────────────────────

void FeatureLua::OnWorldInitialize(WorldRef world)
{
    IFeature::OnWorldInitialize(world);
    detail::TryExecuteLuaFunction(Session, "OnWorldInitialize", world.GetId());
}

void FeatureLua::OnWorldShutdown(WorldRef world)
{
    IFeature::OnWorldShutdown(world);
    detail::TryExecuteLuaFunction(Session, "OnWorldShutdown", world.GetId());
}

void FeatureLua::OnPreWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    IFeature::OnPreWorldUpdate(world, args);
    if (m_luaRuntime) m_luaRuntime->SetCurrentWorld(&world);
    detail::TryExecuteLuaFunction(Session, "OnPreWorldUpdate", world.GetId());
    if (m_luaRuntime) m_luaRuntime->SetCurrentWorld(nullptr);
}

void FeatureLua::OnWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    IFeature::OnWorldUpdate(world, args);

    if (m_luaRuntime) m_luaRuntime->SetCurrentWorld(&world);

    // Drain the thread-safe script queue
    {
        std::vector<std::string> pending;
        {
            std::lock_guard lock(m_scriptQueueMutex);
            pending.swap(m_scriptQueue);
        }

        if (!pending.empty())
        {
            BlockBuffer& buffer = Session->GetBuffer();
            FeatureLuaDynamicBlock* dynamicBlock = buffer.GetBlock<FeatureLuaDynamicBlock>();
            if (dynamicBlock)
            {
                sol::state& state = dynamicBlock->State;
                for (const std::string& code : pending)
                {
                    auto result = state.safe_script(code, sol::script_pass_on_error);
                    if (!result.valid())
                    {
                        sol::error err = result;
                        std::cerr << "Lua script error: " << err.what() << "\n";
                    }
                }
            }
        }
    }

    detail::TryExecuteLuaFunction(Session, "OnWorldUpdate", world.GetId());

    if (m_luaRuntime) m_luaRuntime->SetCurrentWorld(nullptr);
}

void FeatureLua::OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    IFeature::OnPostWorldUpdate(world, args);
    if (m_luaRuntime) m_luaRuntime->SetCurrentWorld(&world);
    detail::TryExecuteLuaFunction(Session, "OnPostWorldUpdate", world.GetId());
    if (m_luaRuntime) m_luaRuntime->SetCurrentWorld(nullptr);
}

bool FeatureLua::OnPreHandleWorldAction(WorldRef world, const FeatureActionArgs& action)
{
    IFeature::OnPreHandleWorldAction(world, action);
    if (m_luaRuntime) m_luaRuntime->SetCurrentWorld(&world);
    TOptional<bool> result = detail::TryExecuteLuaFunctionRet<bool>(Session, "OnPreHandleWorldAction", world.GetId(), action.Action);
    if (m_luaRuntime) m_luaRuntime->SetCurrentWorld(nullptr);
    return result.GetValue(false);
}

bool FeatureLua::OnHandleWorldAction(WorldRef world, const FeatureActionArgs& action)
{
    IFeature::OnHandleWorldAction(world, action);
    if (m_luaRuntime) m_luaRuntime->SetCurrentWorld(&world);
    TOptional<bool> result = detail::TryExecuteLuaFunctionRet<bool>(Session, "OnHandleWorldAction", world.GetId(), action.Action);
    if (m_luaRuntime) m_luaRuntime->SetCurrentWorld(nullptr);
    return result.GetValue(false);
}

bool FeatureLua::OnPostHandleWorldAction(WorldRef world, const FeatureActionArgs& action)
{
    IFeature::OnPostHandleWorldAction(world, action);
    if (m_luaRuntime) m_luaRuntime->SetCurrentWorld(&world);
    TOptional<bool> result = detail::TryExecuteLuaFunctionRet<bool>(Session, "OnPostHandleWorldAction", world.GetId(), action.Action);
    if (m_luaRuntime) m_luaRuntime->SetCurrentWorld(nullptr);
    return result.GetValue(false);
}
