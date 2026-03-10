#include "FeatureString.h"

#include "StringService.h"
#include "PhoenixSim/Session.h"
#include "PhoenixSim/Worlds.h"

Phoenix::FeatureStringDynamicBlock::FeatureStringDynamicBlock(BlockBufferAllocator& allocator, const Config& config)
    : StringTable(allocator, { config.MaxNumStrings, config.MaxBufferCapacity })
{
}

Phoenix::FeatureStringDynamicBlock::FeatureStringDynamicBlock(
    BlockBufferAllocator& allocator,
    const Config& config,
    const FeatureStringDynamicBlock& other)
    : StringTable(allocator, { config.MaxNumStrings, config.MaxBufferCapacity }, other.StringTable)
{
}

Phoenix::BufferBlockLayout Phoenix::FeatureStringDynamicBlock::Layout(Config config)
{
    BufferBlockLayout layout;
    layout.BlockSize = sizeof(FeatureStringDynamicBlock);
    layout.AllocSize += FixedStringTable::GetAllocSizeBytes({ config.MaxNumStrings, config.MaxBufferCapacity });
    return layout;
}

void Phoenix::FeatureStringDynamicBlock::Construct(void* dest, BlockBufferAllocator& allocator, Config config)
{
    new (dest) FeatureStringDynamicBlock(allocator, config);
}

bool Phoenix::FeatureString::Contains(SessionConstRef session, const FName& name)
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

bool Phoenix::FeatureString::Contains(WorldConstRef world, const FName& name)
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

const char* Phoenix::FeatureString::Get(SessionConstRef session, const FName& name)
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

const char* Phoenix::FeatureString::Get(WorldConstRef world, const FName& name)
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

const char* Phoenix::FeatureString::Store(SessionRef session, const char* str, uint32 len)
{
    FeatureStringDynamicBlock* block = session.GetBlock<FeatureStringDynamicBlock>();
    return block ? block->StringTable.Store(str, len) : nullptr;
}

const char* Phoenix::FeatureString::Store(WorldRef world, const char* str, uint32 len)
{
    FeatureStringDynamicBlock* block = world.GetBlock<FeatureStringDynamicBlock>();
    return block ? block->StringTable.Store(str, len) : nullptr;
}

const char* Phoenix::FeatureString::StoreAs(SessionRef session, const char* str, uint32 len, const FName& name)
{
    FeatureStringDynamicBlock* block = session.GetBlock<FeatureStringDynamicBlock>();
    return block ? block->StringTable.StoreAs(str, len, name) : nullptr;
}

const char* Phoenix::FeatureString::StoreAs(WorldRef world, const char* str, uint32 len, const FName& name)
{
    FeatureStringDynamicBlock* block = world.GetBlock<FeatureStringDynamicBlock>();
    return block ? block->StringTable.StoreAs(str, len, name) : nullptr;
}

void Phoenix::FeatureString::OnSessionLayout(const SessionLayoutContext& context, BlockBufferLayoutBuilder& builder)
{
    IFeature::OnSessionLayout(context, builder);

    FeatureStringDynamicBlock::Config dynamicBlockConfig;
    dynamicBlockConfig.MaxNumStrings = 8192;
    dynamicBlockConfig.MaxBufferCapacity = 8192 * 64;

    if (const FeatureJsonConfig* featureConfig = context.Config.GetFeatureConfig(StaticTypeName))
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

void Phoenix::FeatureString::OnWorldLayout(const WorldLayoutContext& context, BlockBufferLayoutBuilder& builder)
{
    IFeature::OnWorldLayout(context, builder);

    FeatureStringDynamicBlock::Config dynamicBlockConfig;
    dynamicBlockConfig.MaxNumStrings = 0;
    dynamicBlockConfig.MaxBufferCapacity = 0;

    if (const FeatureJsonConfig* featureConfig = context.Config.GetFeatureConfig(StaticTypeName))
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
