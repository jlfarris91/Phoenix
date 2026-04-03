#include "PhoenixScript/FeatureScript.h"

#include <filesystem>

#include "WasmRuntime.h"
#include "PhoenixSim/Logging.h"
#include "PhoenixSim/Session.h"
#include "PhoenixSim/Worlds.h"

using namespace Phoenix;

FeatureScriptDynamicBlock::FeatureScriptDynamicBlock(BlockBufferAllocator& allocator, const Config& config)
    : WasmMemory(allocator, config.WasmMemoryCapacity)
{
}

BufferBlockLayout FeatureScriptDynamicBlock::Layout(Config config)
{
    BufferBlockLayout layout;
    layout.BlockSize = sizeof(FeatureScriptDynamicBlock);
    layout.AllocSize += TFixedStorage<uint8>::GetAllocSizeBytes(config.WasmMemoryCapacity);
    return layout;
}

void FeatureScriptDynamicBlock::Construct(void* dest, BlockBufferAllocator& allocator, Config config)
{
    new (dest) FeatureScriptDynamicBlock(allocator, config);
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
            rt->CallVoid("OnPreUpdate");
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
            rt->CallVoid("OnUpdate");
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
            rt->CallVoid("OnPostUpdate");
        }
    }
}

void FeatureScript::OnWorldLayout(const WorldLayoutContext& context, BlockBufferLayoutBuilder& builder)
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
            existing->CallVoid("OnWorldInitialize");
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
        environment->CallVoid("OnWorldInitialize");
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
            it->second->CallVoid("OnWorldShutdown");
        }
        Environments.erase(it);
    }
    IFeature::OnWorldShutdown(world);
}

void FeatureScript::OnPreWorldUpdate(WorldRef world, const FeatureUpdateArgs& /*args*/)
{
    if (auto* rt = GetEnvironment(world); rt && rt->IsValid())
    {
        rt->CallVoid("OnPreWorldUpdate");
    }
}

void FeatureScript::OnWorldUpdate(WorldRef world, const FeatureUpdateArgs& /*args*/)
{
    if (auto* rt = GetEnvironment(world); rt && rt->IsValid())
    {
        rt->CallVoid("OnWorldUpdate");
    }
}

void FeatureScript::OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& /*args*/)
{
    if (auto* rt = GetEnvironment(world); rt && rt->IsValid())
    {
        rt->CallVoid("OnPostWorldUpdate");
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
