
#include "FeatureBlackboard.h"

#include "PhoenixSim/Session.h"
#include "PhoenixSim/Worlds.h"

using namespace Phoenix;
using namespace Phoenix::Blackboard;

FeatureBlackboard::FeatureBlackboard()
{
    FEATURE_SESSION_BLOCK(FeatureBlackboardDynamicSessionBlock)
    FEATURE_WORLD_BLOCK(FeatureBlackboardDynamicWorldBlock)
    FEATURE_CHANNEL(FeatureChannels::PostWorldUpdate)
}

void FeatureBlackboard::OnPostUpdate(const FeatureUpdateArgs& args)
{
    IFeature::OnPostUpdate(args);
    
    FeatureBlackboardDynamicSessionBlock& block = Session->GetBlockRef<FeatureBlackboardDynamicSessionBlock>();
    block.Blackboard.SortAndCompact();
}

void FeatureBlackboard::OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    IFeature::OnPostWorldUpdate(world, args);

    FeatureBlackboardDynamicWorldBlock& block = world.GetBlockRef<FeatureBlackboardDynamicWorldBlock>();
    block.Blackboard.SortAndCompact();
}

SessionBlackboard& FeatureBlackboard::GetGlobalBlackboard(SessionRef session)
{
    FeatureBlackboardDynamicSessionBlock& block = session.GetBlockRef<FeatureBlackboardDynamicSessionBlock>();
    return block.Blackboard;
}

const SessionBlackboard& FeatureBlackboard::GetGlobalBlackboard(SessionConstRef session)
{
    const FeatureBlackboardDynamicSessionBlock& block = session.GetBlockRef<FeatureBlackboardDynamicSessionBlock>();
    return block.Blackboard;
}

WorldBlackboard& FeatureBlackboard::GetBlackboard(WorldRef world)
{
    FeatureBlackboardDynamicWorldBlock& block = world.GetBlockRef<FeatureBlackboardDynamicWorldBlock>();
    return block.Blackboard;
}

const WorldBlackboard& FeatureBlackboard::GetBlackboard(WorldConstRef world)
{
    const FeatureBlackboardDynamicWorldBlock& block = world.GetBlockRef<FeatureBlackboardDynamicWorldBlock>();
    return block.Blackboard;
}
