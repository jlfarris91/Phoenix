#include "Phoenix.Sim.RTS/Effects/Effects.h"

#include "Phoenix.Sim/Session.h"

#include "Phoenix.Sim.RTS/Effects/FeatureEffects.h"

Phoenix::FName Phoenix::RTS::IEffectHandler::GetEffectTypeId() const
{
    return FName::None;
}

void Phoenix::RTS::IEffectHandler::Initialize(const std::shared_ptr<Phoenix::Session>& session)
{
    ISessionService::Initialize(session);
    EffectsFeature = session->GetFeature<FeatureEffects>();
}

void Phoenix::RTS::IEffectHandler::Shutdown()
{
    EffectsFeature.reset();
    ISessionService::Shutdown();
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
