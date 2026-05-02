
#include "PhoenixLua/FeatureLua.h"

#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#else
#include <unistd.h>
#endif

#include "PhoenixLua/LuaWasmEnvironment.h"
#include "PhoenixScript/FeatureScript.h"
#include "PhoenixSim/Logging.h"
#include "PhoenixSim/Session.h"
#include "PhoenixSim/Worlds.h"

using namespace Phoenix;

// Returns the directory containing the running executable.
static std::filesystem::path GetExeDirectory()
{
#ifdef _WIN32
    wchar_t buf[MAX_PATH];
    const DWORD len = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    if (len == 0) return {};
    return std::filesystem::path(buf).parent_path();
#elif defined(__APPLE__)
    char buf[4096];
    uint32_t size = sizeof(buf);
    if (_NSGetExecutablePath(buf, &size) != 0) return {};
    return std::filesystem::canonical(buf).parent_path();
#else
    char buf[4096];
    const ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len <= 0) return {};
    buf[len] = '\0';
    return std::filesystem::path(buf).parent_path();
#endif
}

void FeatureLua::EnqueueScript(std::string code)
{
    std::lock_guard lock(ScriptQueueMutex);
    ScriptQueue.push_back(std::move(code));
}

// ── World lifecycle ───────────────────────────────────────────────────────────

void FeatureLua::OnWorldInitialize(WorldRef world)
{
    IFeature::OnWorldInitialize(world);

    // Read the .lua script path from this world's FeatureLua config.
    std::filesystem::path luaPath;
    if (const FeatureJsonConfig* config = world.GetFeatureConfig(GetFeatureId()))
    {
        const auto& data = config->GetData();
        if (const auto it = data.find("script"); it != data.end())
        {
            namespace fs = std::filesystem;
            luaPath = fs::absolute(
                fs::path(Session->GetWorldsDirectory()) / it->get<std::string>());
        }
    }
    if (luaPath.empty())
        return;

    // lua.wasm lives next to the application executable.
    const std::filesystem::path runnerPath = GetExeDirectory() / "lua.wasm";

    auto featureScript = Session->GetFeature<FeatureScript>();
    if (!featureScript)
    {
        LogError("[FeatureLua] FeatureScript not registered — cannot load lua.wasm");
        return;
    }

    // Create a LuaWasmEnvironment (subclass of WasmEnvironment) for this world.
    // FeatureScript owns it and drives all WASM lifecycle callbacks.
    // FeatureScript::OnWorldInitialize calls the WASM OnWorldInitialize export
    // after this function returns (FeatureLua is registered before FeatureScript).
    LuaWasmEnvironment* luaEnv = featureScript->RegisterWorldRuntime<LuaWasmEnvironment>(world, runnerPath);
    if (!luaEnv)
    {
        LogError("[FeatureLua] Failed to create Lua WASM environment for world using {}",
                 runnerPath.string());
        return;
    }

    if (!luaEnv->LoadLuaScript(luaPath))
    {
        LogError("[FeatureLua] Failed to load Lua script: {}", luaPath.string());
    }
}

void FeatureLua::OnWorldShutdown(WorldRef world)
{
    // Clear any pending scripts for this world so they don't run after shutdown.
    {
        std::lock_guard lock(ScriptQueueMutex);
        ScriptQueue.clear();
    }
    IFeature::OnWorldShutdown(world);
}

void FeatureLua::OnWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    IFeature::OnWorldUpdate(world, args);

    // Drain the thread-safe script queue.
    std::vector<std::string> pending;
    {
        std::lock_guard lock(ScriptQueueMutex);
        pending.swap(ScriptQueue);
    }
    if (pending.empty())
        return;

    auto featureScript = Session->GetFeature<FeatureScript>();
    if (!featureScript)
        return;

    // FeatureScript owns the environment; downcast is safe because FeatureLua
    // always creates LuaWasmEnvironment via RegisterWorldRuntime<LuaWasmEnvironment>.
    auto* luaEnv = static_cast<LuaWasmEnvironment*>(featureScript->GetEnvironment(world));
    if (!luaEnv || !luaEnv->IsValid())
        return;

    for (const std::string& code : pending)
        luaEnv->RunString(code);
}
