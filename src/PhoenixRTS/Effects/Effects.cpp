#include "PhoenixRTS/Effects/Effects.h"

#include "PhoenixSim/Session.h"

#include "PhoenixRTS/Effects/FeatureEffects.h"

Phoenix::FName Phoenix::RTS::IEffectHandler::GetEffectTypeId() const
{
    return FName::None;
}

void Phoenix::RTS::IEffectHandler::Initialize(const TSharedPtr<Phoenix::Session>& session)
{
    IService::Initialize(session);
    EffectsFeature = session->GetFeature<FeatureEffects>();
}

void Phoenix::RTS::IEffectHandler::Shutdown()
{
    IService::Shutdown();
    EffectsFeature.reset();
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
