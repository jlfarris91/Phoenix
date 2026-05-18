#include "FeatureString.h"

#include "StringService.h"
#include "PhoenixSim/Session.h"
#include "PhoenixSim/Worlds.h"

using namespace Phoenix;

void FeatureStringDynamicBlock::Construct(BlockBufferAllocator& allocator, const Config& config)
{
    StringTable.Construct(allocator, config.MaxNumStrings, config.MaxBufferCapacity);
}

BlockBufferLayout FeatureStringDynamicBlock::StaticLayout(const Config& config)
{
    return BlockBufferLayout::For<FeatureStringDynamicBlock>()
        .Container<FixedStringTable>("StringTable", config.MaxNumStrings, config.MaxBufferCapacity);
}

bool FeatureString::Contains(SessionConstRef session, const FName& name)
{
    if (const FeatureStringDynamicBlock* block = session.GetBlock<FeatureStringDynamicBlock>())
    {
        if (block->StringTable.Contains(name))
        {
            return true;
        }
    }
    // Fallback to the static string service
    return GetStringService().Get(name) != nullptr;
}

bool FeatureString::Contains(WorldConstRef world, const FName& name)
{
    if (const FeatureStringDynamicBlock* block = world.GetBlock<FeatureStringDynamicBlock>())
    {
        if (block->StringTable.Contains(name))
        {
            return true;
        }
    }
    if (std::shared_ptr<Phoenix::Session> session = world.GetSession().lock())
    {
        return Contains(*session, name);
    }
    // Fallback to the static string service
    return GetStringService().Get(name) != nullptr;
}

const char* FeatureString::Get(SessionConstRef session, const FName& name)
{
    if (const FeatureStringDynamicBlock* block = session.GetBlock<FeatureStringDynamicBlock>())
    {
        if (const char* str = block->StringTable.Get(name))
        {
            return str;
        }
    }
    // Fallback to the static string service
    return GetStringService().Get(name);
}

const char* FeatureString::Get(WorldConstRef world, const FName& name)
{
    if (const FeatureStringDynamicBlock* block = world.GetBlock<FeatureStringDynamicBlock>())
    {
        if (const char* str = block->StringTable.Get(name))
        {
            return str;
        }
    }
    if (std::shared_ptr<Phoenix::Session> session = world.GetSession().lock())
    {
        return Get(*session, name);
    }
    // Fallback to the static string service
    return GetStringService().Get(name);
}

const char* FeatureString::Store(SessionRef session, const char* str, uint32 len)
{
    FeatureStringDynamicBlock* block = session.GetBlock<FeatureStringDynamicBlock>();
    return block ? block->StringTable.Store(str, len) : nullptr;
}

const char* FeatureString::Store(WorldRef world, const char* str, uint32 len)
{
    FeatureStringDynamicBlock* block = world.GetBlock<FeatureStringDynamicBlock>();
    return block ? block->StringTable.Store(str, len) : nullptr;
}

const char* FeatureString::StoreAs(SessionRef session, const char* str, uint32 len, const FName& name)
{
    FeatureStringDynamicBlock* block = session.GetBlock<FeatureStringDynamicBlock>();
    return block ? block->StringTable.StoreAs(str, len, name) : nullptr;
}

const char* FeatureString::StoreAs(WorldRef world, const char* str, uint32 len, const FName& name)
{
    FeatureStringDynamicBlock* block = world.GetBlock<FeatureStringDynamicBlock>();
    return block ? block->StringTable.StoreAs(str, len, name) : nullptr;
}

void FeatureString::OnSessionLayout(const SessionLayoutContext& context, BlockBufferConfigBuilder& builder)
{
    IFeature::OnSessionLayout(context, builder);

    FeatureStringDynamicBlock::Config dynamicBlockConfig;
    dynamicBlockConfig.MaxNumStrings = 8192;
    dynamicBlockConfig.MaxBufferCapacity = 8192 * 64;

    if (const FeatureJsonConfig* featureConfig = context.Config.GetFeatureConfig(GetFeatureId()))
    {
        const nlohmann::json& featureConfigData = featureConfig->GetData();

        dynamicBlockConfig.MaxNumStrings = featureConfigData.value("max_num_strings", dynamicBlockConfig.MaxNumStrings);
        dynamicBlockConfig.MaxBufferCapacity = featureConfigData.value("max_buffer_capacity", dynamicBlockConfig.MaxBufferCapacity);
    }

    if (dynamicBlockConfig.MaxNumStrings > 0)
    {
        builder.RegisterBlockWithAlloc<FeatureStringDynamicBlock>(EBufferBlockType::Dynamic, dynamicBlockConfig);
    }
}

void FeatureString::OnWorldLayout(const WorldLayoutContext& context, BlockBufferConfigBuilder& builder)
{
    IFeature::OnWorldLayout(context, builder);

    FeatureStringDynamicBlock::Config dynamicBlockConfig;
    dynamicBlockConfig.MaxNumStrings = 0;
    dynamicBlockConfig.MaxBufferCapacity = 0;

    if (const FeatureJsonConfig* featureConfig = context.Config.GetFeatureConfig(GetFeatureId()))
    {
        const nlohmann::json& featureConfigData = featureConfig->GetData();

        dynamicBlockConfig.MaxNumStrings = featureConfigData.value("max_num_strings", dynamicBlockConfig.MaxNumStrings);
        dynamicBlockConfig.MaxBufferCapacity = featureConfigData.value("max_buffer_capacity", dynamicBlockConfig.MaxBufferCapacity);
    }

    if (dynamicBlockConfig.MaxNumStrings > 0)
    {
        builder.RegisterBlockWithAlloc<FeatureStringDynamicBlock>(EBufferBlockType::Dynamic, dynamicBlockConfig);
    }
}
