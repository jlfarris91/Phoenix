#include "Phoenix.Sim.Script/FeatureScript.h"

#include <filesystem>

#include "WasmRuntime.h"
#include "Phoenix.Sim/Logging.h"
#include "Phoenix.Sim/Session.h"
#include "Phoenix.Sim/Worlds.h"

using namespace Phoenix;

void FeatureScriptDynamicBlock::Construct(BlockBufferAllocator& allocator, const Config& config)
{
    WasmMemory.Construct(allocator, config.WasmMemoryCapacity);
}

BlockBufferLayout FeatureScriptDynamicBlock::StaticLayout(const Config& config)
{
    return BlockBufferLayout::For<FeatureScriptDynamicBlock>()
        .Container<TFixedStorage<uint8>>("WasmMemory", config.WasmMemoryCapacity);
}

// ── FeatureScript ─────────────────────────────────────────────────────────────

void FeatureScript::Initialize(const std::shared_ptr<Phoenix::Session>& session)
{
    IFeature::Initialize(session);
}

void FeatureScript::Shutdown()
{
    Runtimes.clear();
    Environments.clear();
    IFeature::Shutdown();
}

void FeatureScript::OnPreUpdate(const FeatureUpdateArgs& args)
{
    IFeature::OnPreUpdate(args);
    for (auto& rt : Environments | std::views::values)
    {
        if (rt && rt->IsValid())
        {
            rt->CallExport("OnPreUpdate", 0, nullptr, 0, nullptr);
        }
    }
}

void FeatureScript::OnUpdate(const FeatureUpdateArgs& args)
{
    IFeature::OnUpdate(args);
    for (auto& rt : Environments | std::views::values)
    {
        if (rt && rt->IsValid())
        {
            rt->CallExport("OnUpdate", 0, nullptr, 0, nullptr);
        }
    }
}

void FeatureScript::OnPostUpdate(const FeatureUpdateArgs& args)
{
    IFeature::OnPostUpdate(args);
    for (auto& rt : Environments | std::views::values)
    {
        if (rt && rt->IsValid())
        {
            rt->CallExport("OnPostUpdate", 0, nullptr, 0, nullptr);
        }
    }
}

void FeatureScript::OnWorldLayout(const WorldLayoutContext& context, BlockBufferConfigBuilder& builder)
{
    IFeature::OnWorldLayout(context, builder);

    FeatureScriptDynamicBlock::Config dynamicBlockConfig;

    if (const FeatureJsonConfig* featureConfig = context.Config.GetFeatureConfig(GetFeatureId()))
    {
        const nlohmann::json& featureConfigData = featureConfig->GetData();
        dynamicBlockConfig.WasmMemoryCapacity = featureConfigData.value("memory_capacity", dynamicBlockConfig.WasmMemoryCapacity);
    }

    builder.RegisterBlockWithAlloc<FeatureScriptDynamicBlock>(EBufferBlockType::Dynamic, dynamicBlockConfig);
}


void FeatureScript::OnWorldInitialize(WorldRef world)
{
    IFeature::OnWorldInitialize(world);

    // If another feature (e.g. FeatureLua) pre-created an environment for this
    // world via RegisterWorldRuntime, just fire the lifecycle callback on it.
    if (WasmEnvironment* existing = GetEnvironment(world))
    {
        if (existing->IsValid())
        {
            existing->CallExport("OnWorldInitialize", 0, nullptr, 0, nullptr);
        }
        return;
    }

    // Otherwise check for a FeatureScript-specific "script" world config.
    std::filesystem::path scriptPath;
    if (const FeatureJsonConfig* config = world.GetFeatureConfig(GetFeatureId()))
    {
        const auto& data = config->GetData();
        if (const auto it = data.find("script"); it != data.end())
        {
            namespace fs = std::filesystem;
            scriptPath = fs::absolute(fs::path(Session->GetWorldsDirectory()) / it->get<std::string>());
        }
    }
    if (scriptPath.empty())
        return;

    auto runtime = GetOrLoadWasmRuntime(scriptPath);
    if (!runtime)
        return;

    auto environment = std::make_unique<WasmEnvironment>(Session.get(), &world, runtime);
    if (environment->IsValid())
    {
        environment->CallExport("OnWorldInitialize", 0, nullptr, 0, nullptr);
    }

    Environments.emplace(world.GetId(), std::move(environment));
}

void FeatureScript::OnWorldShutdown(WorldRef world)
{
    const hash32_t key = world.GetId();
    auto it = Environments.find(key);
    if (it != Environments.end())
    {
        if (it->second && it->second->IsValid())
        {
            it->second->CallExport("OnWorldShutdown", 0, nullptr, 0, nullptr);
        }
        Environments.erase(it);
    }
    IFeature::OnWorldShutdown(world);
}

void FeatureScript::OnPreWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    if (auto* env = GetEnvironment(world); env && env->IsValid())
    {
        const float dt = 1.0f / Time::D;
        const void* argPtrs[] = { &dt };
        env->CallExport("OnPreWorldUpdate", 1, argPtrs, 0, nullptr);
    }
}

void FeatureScript::OnWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    if (auto* env = GetEnvironment(world); env && env->IsValid())
    {
        const float dt = 1.0f / Time::D;
        const void* argPtrs[] = { &dt };
        env->CallExport("OnWorldUpdate", 1, argPtrs, 0, nullptr);
    }
}

void FeatureScript::OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    if (auto* env = GetEnvironment(world); env && env->IsValid())
    {
        const float dt = 1.0f / Time::D;
        const void* argPtrs[] = { &dt };
        env->CallExport("OnPostWorldUpdate", 1, argPtrs, 0, nullptr);
    }
}

std::shared_ptr<WasmRuntime> FeatureScript::GetRuntime(const std::string& fileName) const
{
    auto it = Runtimes.find(fileName);
    return (it != Runtimes.end()) ? it->second : nullptr;
}

std::shared_ptr<WasmRuntime> FeatureScript::GetOrLoadWasmRuntime(const std::filesystem::path& path)
{
    if (auto existingRuntime = GetRuntime(path.generic_string()))
    {
        return existingRuntime;
    }

    auto runtime = std::make_shared<WasmRuntime>(Session);

    if (!runtime->LoadFile(path.generic_string().c_str()))
    {
        LogError("Failed to load WASM runtime from path {}", path.generic_string());
        return nullptr;
    }

    auto relativePath = std::filesystem::relative(path, Session->GetDataDirectory());
    Runtimes.emplace(relativePath.generic_string(), runtime);

    return runtime;
}

WasmEnvironment* FeatureScript::GetEnvironment(WorldRef world) const
{
    auto it = Environments.find(world.GetId());
    return (it != Environments.end()) ? it->second.get() : nullptr;
}
