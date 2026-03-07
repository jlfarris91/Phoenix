
#include "FeatureBlackboard.h"

#include "PhoenixSim/Session.h"
#include "PhoenixSim/Worlds.h"

using namespace Phoenix;
using namespace Phoenix::Blackboard;

FeatureBlackboardBlock::FeatureBlackboardBlock(BlockBufferAllocator& allocator, const Config& config)
    : Blackboard(allocator, config.MaxBlackboardItems)
{
}

FeatureBlackboardBlock::FeatureBlackboardBlock(
    BlockBufferAllocator& allocator,
    const Config& config,
    const FeatureBlackboardBlock& other)
    : Blackboard(allocator, config.MaxBlackboardItems, other.Blackboard)
{
}

BufferBlockLayout FeatureBlackboardBlock::Layout(Config config)
{
    BufferBlockLayout layout;
    layout.BlockSize = sizeof(FeatureBlackboardBlock);
    layout.AllocSize = FixedBlackboard::GetAllocSizeBytes(config.MaxBlackboardItems);
    return layout;
}

void FeatureBlackboardBlock::Construct(void* dest, BlockBufferAllocator& allocator, Config config)
{
    new (dest) FeatureBlackboardBlock(allocator, config);
}

FeatureBlackboard::FeatureBlackboard()
{
    FEATURE_CHANNEL(FeatureChannels::PostWorldUpdate)
}

void FeatureBlackboard::OnWorldLayout(const WorldLayoutContext& context, BlockBufferLayoutBuilder& builder)
{
    IFeature::OnWorldLayout(context, builder);

    FeatureBlackboardBlock::Config blockConfig;
    blockConfig.MaxBlackboardItems = PHX_BLACKBOARD_MAX_WORLD_SIZE;

    if (const FeatureJsonConfig* featureConfig = context.Config.GetFeatureConfig(StaticTypeName))
    {
        const nlohmann::json& featureConfigData = featureConfig->GetData();
        blockConfig.MaxBlackboardItems = featureConfigData.value("max_blackboard_items", blockConfig.MaxBlackboardItems);
    }

    builder.RegisterBlockWithAlloc<FeatureBlackboardBlock>(EBufferBlockType::Dynamic, blockConfig);
}

void FeatureBlackboard::OnPostUpdate(const FeatureUpdateArgs& args)
{
    IFeature::OnPostUpdate(args);

    FeatureBlackboardBlock& block = Session->GetBlockRef<FeatureBlackboardBlock>();

    {
        PHX_PROFILE_ZONE_SCOPED_N("SortBlackboard");
        block.Blackboard.Sort();
    }
}

void FeatureBlackboard::OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    IFeature::OnPostWorldUpdate(world, args);

    FeatureBlackboardBlock& block = world.GetBlockRef<FeatureBlackboardBlock>();
    block.Blackboard.Sort();
}

FixedBlackboard& FeatureBlackboard::GetGlobalBlackboard(SessionRef session)
{
    FeatureBlackboardBlock& block = session.GetBlockRef<FeatureBlackboardBlock>();
    return block.Blackboard;
}

const FixedBlackboard& FeatureBlackboard::GetGlobalBlackboard(SessionConstRef session)
{
    const FeatureBlackboardBlock& block = session.GetBlockRef<FeatureBlackboardBlock>();
    return block.Blackboard;
}

FixedBlackboard& FeatureBlackboard::GetBlackboard(WorldRef world)
{
    FeatureBlackboardBlock& block = world.GetBlockRef<FeatureBlackboardBlock>();
    return block.Blackboard;
}

const FixedBlackboard& FeatureBlackboard::GetBlackboard(WorldConstRef world)
{
    const FeatureBlackboardBlock& block = world.GetBlockRef<FeatureBlackboardBlock>();
    return block.Blackboard;
}
