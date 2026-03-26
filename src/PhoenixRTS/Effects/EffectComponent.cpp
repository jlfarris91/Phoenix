#include "PhoenixRTS/Effects/EffectComponent.h"
#include "PhoenixSim/Reflection/Registration.h"

using namespace Phoenix;
using namespace Phoenix::RTS;

PHX_DEFINE_TYPE(EffectComponent)
{
    registration
        .Field("Name",            &EffectComponent::Name)
        .Field("SourceId",        &EffectComponent::SourceId)
        .Field("TargetId",        &EffectComponent::TargetId)
        .Field("SourcePos",       &EffectComponent::SourcePos)
        .Field("TargetPos",       &EffectComponent::TargetPos)
        .Field("RefCount",        &EffectComponent::RefCount)
        .Field("ChannelingCount", &EffectComponent::ChannelingCount);
}
