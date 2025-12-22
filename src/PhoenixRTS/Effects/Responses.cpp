#include "PhoenixRTS/Effects/Responses.h"

#include "PhoenixSim/ECS/FeatureECS.h"

#include "PhoenixRTS/Cooldown/Cooldown.h"
#include "PhoenixRTS/Data/DataResponse.h"
#include "PhoenixRTS/Effects/EffectComponent.h"
#include "PhoenixRTS/Effects/FeatureEffects.h"
#include "PhoenixRTS/TargetFiltering/TargetFiltering.h"

using namespace Phoenix;
using namespace Phoenix::RTS;

void IResponseHandler::Initialize(SessionRef session)
{
}

void IResponseHandler::Shutdown(SessionRef session)
{
}

void IResponseHandler::OnWorldInitialize(WorldRef world)
{
}

void IResponseHandler::OnWorldShutdown(WorldRef world)
{
}

FName IResponseHandler::GetResponseId() const
{
    return FName::None;
}

int32 IResponseHandler::GetPriority(WorldConstRef world, const ResponseContext& context) const
{
    return 0;
}

bool IResponseHandler::Execute(WorldRef world, const ResponseContext& context) const
{
    return false;
}

bool IResponseHandler::CanExecute(WorldConstRef world, const ResponseContext& context) const
{
    return false;
}

ResponseHandlerBase::ResponseHandlerBase()
    :ResponseId("Response"_n)
{
}

ResponseHandlerBase::ResponseHandlerBase(const FName& responseId)
    : ResponseId(responseId)
{
}

void ResponseHandlerBase::Initialize(SessionRef session)
{
    EffectsFeature = session.GetFeature<FeatureEffects>();
}

void ResponseHandlerBase::Shutdown(SessionRef session)
{
    EffectsFeature.reset();
}

FName ResponseHandlerBase::GetResponseId() const
{
    return ResponseId;
}

int32 ResponseHandlerBase::GetPriority(WorldConstRef world, const ResponseContext& context) const
{
    Data::ResponsePtr dataPtr(context.ResponseId);
    return dataPtr.Priority().GetValue(*context.LdsQueryContext);
}

bool ResponseHandlerBase::Execute(WorldRef world, const ResponseContext& context) const
{
    const LDS::ILDSQueryContext& lds = *context.LdsQueryContext;

    Data::ResponsePtr dataPtr(context.ResponseId);

    Data::EResponseTarget responseTarget = dataPtr.Target().GetValue(lds);
    if (context.Target != responseTarget)
    {
        return false;
    }

    FName requiredEffectId = dataPtr.RequiredEffect().GetReferenceId(lds);
    if (requiredEffectId != FName::None && context.EffectComponent->Name != requiredEffectId)
    {
        return false;
    }

    FName responseEffectId = dataPtr.ResponseEffect().GetReferenceId(lds);
    if (responseEffectId == FName::None)
    {
        return false;
    }

    ECS::EntityId source;
    ECS::EntityId target;

    switch (responseTarget)
    {
        case Data::EResponseTarget::Source:
            {
                source = context.EffectComponent->TargetId;
                target = context.EffectComponent->SourceId;
            }
            break;
        case Data::EResponseTarget::Target:
            {
                source = context.EffectComponent->SourceId;
                target = context.EffectComponent->TargetId;
            }
            break;
        case Data::EResponseTarget::None:
            return false;
    }

    if (target != ECS::EntityId::Invalid)
    {
        Data::TargetFilter targetFilter = dataPtr.TargetFilter().ReadObject(lds);
        if (!TargetFiltering::PassesTargetFilter(world, targetFilter, source, target))
        {
            return false;
        }
    }

    Data::Cooldown cooldown = dataPtr.Cooldown().ReadObject(lds);
    if (Cooldown::IsCooldownActive(world, source, context.ResponseId, cooldown))
    {
        return false;
    }

    // TODO (jfarris): run validator

    ExecuteEffectArgs overrides;
    overrides.SourceId = source;
    overrides.SourcePos = ECS::FeatureECS::GetWorldPosition(world, source);
    overrides.TargetId = target;
    overrides.TargetPos = ECS::FeatureECS::GetWorldPosition(world, target);

    EffectScopeId effectScopeId = EffectsFeature->GetEffectScope(world, context.EffectNodeId);

    bool useTempScope = effectScopeId == EffectScopeId::Invalid;
    if (useTempScope)
    {
        effectScopeId = EffectsFeature->AcquireEffectScope(world, overrides);
    }

    EffectsFeature->ExecuteEffect(world, effectScopeId, responseEffectId, overrides);

    // TODO (jfarris): spawn fx actor

    if (useTempScope)
    {
        EffectsFeature->ReleaseEffectScope(world, effectScopeId);
    }

    return true;
}

bool ResponseHandlerBase::CanExecute(WorldConstRef world, const ResponseContext& context) const
{
    return true;
}
