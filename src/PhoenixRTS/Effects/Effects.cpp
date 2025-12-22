#include "PhoenixRTS/Effects/Effects.h"

#include "PhoenixRTS/Effects/FeatureEffects.h"

Phoenix::FName Phoenix::RTS::IEffectHandler::GetEffectId() const
{
    return FName::None;
}

void Phoenix::RTS::IEffectHandler::Initialize(SessionRef session)
{
}

void Phoenix::RTS::IEffectHandler::Shutdown(SessionRef session)
{
}

void Phoenix::RTS::IEffectHandler::OnWorldInitialize(WorldRef world)
{
}

void Phoenix::RTS::IEffectHandler::OnWorldShutdown(WorldRef world)
{
}

bool Phoenix::RTS::IEffectHandler::Execute(WorldRef world, const EffectExecuteContext& context) const
{
    return false;
}

bool Phoenix::RTS::IEffectHandler::CanExecute(WorldConstRef world, const EffectExecuteContext& context) const
{
    return false;
}

bool Phoenix::RTS::IEffectHandler::Finalize(WorldRef world, const EffectFinalizeContext& context) const
{
    return false;
}

Phoenix::RTS::EffectHandlerBase::EffectHandlerBase(const FName& effectId)
    : EffectId(effectId)
{
}

Phoenix::FName Phoenix::RTS::EffectHandlerBase::GetEffectId() const
{
    return EffectId;
}

void Phoenix::RTS::EffectHandlerBase::Initialize(SessionRef session)
{
    EffectsFeature = session.GetFeature<FeatureEffects>();
}

void Phoenix::RTS::EffectHandlerBase::Shutdown(SessionRef session)
{
    EffectsFeature.reset();
}

bool Phoenix::RTS::EffectHandlerBase::Execute(WorldRef world, const EffectExecuteContext& context) const
{
    return false;
}

bool Phoenix::RTS::EffectHandlerBase::CanExecute(WorldConstRef world, const EffectExecuteContext& context) const
{
    return true;
}
