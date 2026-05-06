
#include "FeatureBlackboard.h"

#include "PhoenixSim/Session.h"
#include "PhoenixSim/Worlds.h"
#include "PhoenixSim/BlockBuffer/BlockBufferAllocator.h"
#include "PhoenixSim/BlockBuffer/BlockBufferLayout.h"

using namespace Phoenix;
using namespace Phoenix::Blackboard;

void FeatureBlackboardBlock::Construct(BlockBufferAllocator& allocator, const Config& config)
{
    Blackboard.Construct(allocator, config.MaxBlackboardItems);
}

BlockBufferLayout FeatureBlackboardBlock::StaticLayout(const Config& config)
{
    return BlockBufferLayout::For<FeatureBlackboardBlock>()
        .Container<FixedBlackboard>("Blackboard", config.MaxBlackboardItems);
}

void FeatureBlackboard::OnWorldLayout(const WorldLayoutContext& context, BlockBufferConfigBuilder& builder)
{
    IFeature::OnWorldLayout(context, builder);

    FeatureBlackboardBlock::Config blockConfig;
    blockConfig.MaxBlackboardItems = PHX_BLACKBOARD_MAX_WORLD_SIZE;

    if (const FeatureJsonConfig* featureConfig = context.Config.GetFeatureConfig(GetFeatureId()))
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
