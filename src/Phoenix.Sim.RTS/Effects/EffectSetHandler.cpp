#include "Phoenix.Sim.RTS/Effects/EffectSetHandler.h"

#include "Phoenix.Sim/LDS/FeatureLDS.h"

#include "Phoenix.Sim.RTS/Data/DataEffectSet.h"
#include "Phoenix.Sim.RTS/Effects/FeatureEffects.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

FName EffectSetHandler::GetEffectTypeId() const
{
    return "EffectSet"_n;
}

bool EffectSetHandler::Execute(WorldRef world, const EffectExecuteContext& context) const
{
    const LDS::ILDSQueryContext& lds = *context.LdsQueryContext;

    EffectNodeId node = FeatureEffects::AcquireEffectNode(world, context.ParentId, *context.ParentComponent);

    Data::EffectSetPtr effectSet(context.EffectId);

    // TODO (jfarris): validate the effect
    // if (FeatureEffects::ValidateEffectNode())
    {
        effectSet.Effects().ForEachItem(lds, [&](uint32, const LDS::LDSObjectRefPtr& effectPtr)
        {
            FName effectId = effectPtr.GetReferenceId(lds);
            if (FName::IsNoneOrEmpty(effectId))
            {
                return;
            }

            EffectsFeature->ExecuteEffect(world, node, effectId);
        });
    }

    FeatureEffects::DereferenceEffectNode(world, node);

    return true;
}

bool EffectSetHandler::CanExecute(WorldConstRef world, const EffectExecuteContext& context) const
{
    return true;
}
