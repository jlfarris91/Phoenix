#include "Phoenix.Sim.RTS/Effects/ResponseDamageHandler.h"

#include "Phoenix.Sim/ECS/FeatureECS.h"
#include "Phoenix.Sim/LDS/FeatureLDS.h"

#include "Phoenix.Sim.RTS/Data/DataResponseDamage.h"
#include "Phoenix.Sim.RTS/Effects/FeatureEffects.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

ResponseDamageHandler::ResponseDamageHandler()
    : ResponseHandlerBase("ResponseDamage"_n)
{
}

bool ResponseDamageHandler::Execute(WorldRef world, const ResponseContext& context) const
{
    const LDS::ILDSQueryContext& lds = *context.LdsQueryContext;

    Value damage = FeatureEffects::GetEffectDamage(world, context.EffectNodeId);

    Data::ResponseDamagePtr dataPtr(context.ResponseId);

    Value clamp;
    if (dataPtr.Clamp().TryGetValue(lds, clamp) && clamp > 0 && damage > clamp)
    {
        Value clampRemainder = damage - clamp;
        Value clampRemainderScalar = dataPtr.ClampRemainderScalar().GetValue(lds);
        damage = clamp + clampRemainder * clampRemainderScalar;
    }

    // TODO (jfarris): do we always want to apply flat damage reduction before scalar?
    damage += dataPtr.Amount().GetValue(lds);

    Value amountScalar;
    if (dataPtr.AmountScalar().TryGetValue(lds, amountScalar))
    {
        damage *= amountScalar;
    }

    FeatureEffects::SetEffectDamage(world, context.EffectNodeId, damage);

    return ResponseHandlerBase::Execute(world, context);
}
