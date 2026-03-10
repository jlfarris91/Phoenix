
#include "PhoenixLua/FeatureLua.h"

#include <iostream>
#include <lua.h>

#include "PhoenixSim/Flags.h"
#include "PhoenixSim/Session.h"
#include "PhoenixSim/Containers/Optional.h"
#include "PhoenixSim/ECS/FeatureECS.h"

#include "PhoenixPhysics/BodyComponent.h"
#include "PhoenixPhysics/FeaturePhysics.h"

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

void FeatureLua::Initialize(const std::shared_ptr<Phoenix::Session>& session)
{
    IFeature::Initialize(session);

    BlockBuffer& buffer = Session->GetBuffer();
    FeatureLuaDynamicBlock* dynamicBlock = buffer.GetBlock<FeatureLuaDynamicBlock>();
    if (!dynamicBlock)
    {
        return;
    }

    sol::state& state = dynamicBlock->State;

    state.set_exception_handler([] (lua_State*, sol::optional<const std::exception&> ex, sol::string_view v)
    {
        if (ex.has_value())
        {
            std::cout << "Lua error caught in C++: " << ex->what() << std::endl;
        }
        else
        {
            std::cout << "Lua error caught in C++: " << v << std::endl;
        }
        return 0;
    });

    state.open_libraries(sol::lib::base);

    Lua::LuaOpen_FP64(state);

    auto phoenix = state["Phoenix"].get_or_create<sol::table>();

    state["hash"] = [](const std::string& v)
    {
        return FName(Hashing::FNV1A32(v.data(), v.size()));
    };

    auto ecs = phoenix["ECS"].get_or_create<sol::table>();

    ecs["AcquireEntity"] = [this](const FName& worldName, const FName& kind)
    {
        if (auto world = Session->GetWorldManager()->GetWorld(worldName))
        {
            return ECS::FeatureECS::StaticAcquireEntity(*world, kind);
        }
        return ECS::EntityId::Invalid;
    };

    ecs["ReleaseEntity"] = [this](FName worldName, ECS::EntityId entityId)
    {
        if (auto world = Session->GetWorldManager()->GetWorld(worldName))
        {
            return ECS::FeatureECS::StaticReleaseEntity(*world, entityId);
        }
        return false;
    };

    auto unit = phoenix["Unit"].get_or_create<sol::table>();

    unit["SpawnUnit"] = [this](const FName& worldName, size_t count, const FName& kind, Distance x, Distance y, Angle facing) -> size_t
    {
        auto world = Session->GetWorldManager()->GetWorld(worldName);
        if (!world)
        {
            return 0;
        }

        size_t i = 0;
        for (; i < count; ++i)
        {
            ECS::EntityId entityId = ECS::FeatureECS::StaticAcquireEntity(*world, kind);
            if (entityId == ECS::EntityId::Invalid)
                break;

            if (ECS::TransformComponent* transformComp = ECS::FeatureECS::AddComponent<ECS::TransformComponent>(*world, entityId))
            {
                transformComp->Transform.Position.X = x;
                transformComp->Transform.Position.Y = y;
                transformComp->Transform.Rotation = facing;
            }

            if (Physics::BodyComponent* bodyComp = ECS::FeatureECS::AddComponent<Physics::BodyComponent>(*world, entityId))
            {
                bodyComp->CollisionMask = 1;
                bodyComp->Radius = 0.6; // Lancer :)
                bodyComp->InvMass = OneDivBy<Value>(1.0f);
                bodyComp->LinearDamping = 5.f;
                SetFlagRef(bodyComp->Flags, Physics::EBodyFlags::Awake, true);
            }
        }

        return i;
    };

    auto physics = phoenix["Physics"].get_or_create<sol::table>();

    physics["ApplyForce"] = [this](const FName& worldName, ECS::EntityId entityId, Distance forceX, Distance forceY)
    {
        auto world = Session->GetWorldManager()->GetWorld(worldName);
        if (!world)
        {
            return 0;
        }

        Physics::FeaturePhysics::AddForce(*world, entityId, {forceX, forceY});
        return 1;
    };

    std::string fileName = R"(.\Tests\TestApp\Maps\TestMap\test_script.lua)";
    auto script = state.load_file(fileName);

    if (!script.valid())
    {
        sol::error err = script;
        std::cerr << "Error loading script: " << err.what() << std::endl;
        return;
    }

    auto result = script();

    if (!result.valid())
    {
        sol::error err = script;
        std::cerr << "Error loading script: " << err.what() << std::endl;
#ifdef _WIN32
        __debugbreak();
#else
        __builtin_trap();  // GCC/Clang equivalent
#endif
    }
}

void FeatureLua::Shutdown()
{
    IFeature::Shutdown();
}

namespace detail
{
    template <class TRet, class ...TArgs>
    TOptional<TRet> TryExecuteLuaFunctionRet(sol::state& state, const char* funcName, TArgs&& ...args)
    {
        sol::protected_function luaFunc = state[funcName];

        if (!luaFunc.valid())
        {
            std::cout << "Could not find global function named '" << funcName << "'." << std::endl;
            return TOptional<TRet>();
        }

        sol::protected_function_result result = luaFunc(std::forward<TArgs>(args)...);

        if (!result.valid())
        {
            sol::error err = result;
            std::cout << "Lua error caught in C++: " << err.what() << std::endl;
            return TOptional<TRet>();
        }

        if (result.return_count() == 0)
        {
            return TOptional<TRet>();
        }

        TRet returnValue = result;

#if 1
        std::cout << "Function '" << funcName << "' returned: " << returnValue << std::endl;
#endif

        return returnValue;
    }

    template <class ...TArgs>
    bool TryExecuteLuaFunction(sol::state& state, const char* funcName, TArgs&& ...args)
    {
        sol::protected_function luaFunc = state[funcName];

        if (!luaFunc.valid())
        {
            std::cout << "Could not find global function named '" << funcName << "'." << std::endl;
            return false;
        }

        sol::protected_function_result result = luaFunc(std::forward<TArgs>(args)...);

        if (!result.valid())
        {
            sol::error err = result;
            std::cout << "Lua error caught in C++: " << err.what() << std::endl;
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
        {
            return TOptional<TRet>();
        }

        sol::state& lua = dynamicBlock->State;
        return TryExecuteLuaFunctionRet<TRet, TArgs...>(lua, funcName, std::forward<TArgs>(args)...);
    }

    template <class ...TArgs>
    bool TryExecuteLuaFunction(const std::shared_ptr<Session>& session, const char* funcName, TArgs&& ...args)
    {
        BlockBuffer& buffer = session->GetBuffer();
        FeatureLuaDynamicBlock* dynamicBlock = buffer.GetBlock<FeatureLuaDynamicBlock>();
        if (!dynamicBlock)
        {
            return false;
        }

        sol::state& lua = dynamicBlock->State;
        return TryExecuteLuaFunction(lua, funcName, std::forward<TArgs>(args)...);
    }
}

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

    detail::TryExecuteLuaFunction(Session, "OnPreWorldUpdate", world.GetId());
}

void FeatureLua::OnWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    IFeature::OnWorldUpdate(world, args);

    detail::TryExecuteLuaFunction(Session, "OnWorldUpdate", world.GetId());
}

void FeatureLua::OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    IFeature::OnPostWorldUpdate(world, args);

    detail::TryExecuteLuaFunction(Session, "OnPostWorldUpdate", world.GetId());
}

bool FeatureLua::OnPreHandleWorldAction(WorldRef world, const FeatureActionArgs& action)
{
    IFeature::OnPreHandleWorldAction(world, action);

    TOptional<bool> result = detail::TryExecuteLuaFunctionRet<bool>(Session, "OnPreHandleWorldAction", world.GetId(), action.Action);

    return result.GetValue(false);
}

bool FeatureLua::OnHandleWorldAction(WorldRef world, const FeatureActionArgs& action)
{
    IFeature::OnHandleWorldAction(world, action);

    TOptional<bool> result = detail::TryExecuteLuaFunctionRet<bool>(Session, "OnHandleWorldAction", world.GetId(), action.Action);

    return result.GetValue(false);
}

bool FeatureLua::OnPostHandleWorldAction(WorldRef world, const FeatureActionArgs& action)
{
    IFeature::OnPostHandleWorldAction(world, action);

    TOptional<bool> result = detail::TryExecuteLuaFunctionRet<bool>(Session, "OnPostHandleWorldAction", world.GetId(), action.Action);

    return result.GetValue(false);
}
