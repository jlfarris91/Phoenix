#include "PhoenixRTS/Effects/EffectDamageHandler.h"

#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/LDS/FeatureLDS.h"

#include "PhoenixRTS/Data/DataEffectDamage.h"
#include "PhoenixRTS/Effects/EffectComponent.h"
#include "PhoenixRTS/Effects/FeatureEffects.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

EffectDamageHandler::EffectDamageHandler()
    : EffectHandlerBase("EffectDamage"_n)
{
}

bool EffectDamageHandler::Execute(WorldRef world, const EffectContext& context) const
{
    const LDS::ILDSQueryContext& lds = *context.LdsQueryContext;

    EffectNodeId nodeId = FeatureEffects::AcquireEffectNode(world, context.ParentId, *context.Parent);

    EffectComponent* nodeComp = FeatureEffects::GetEffectComponent(world, nodeId);
    if (!nodeComp)
    {
        return false;
    }

    Data::EffectDamagePtr effectDamage(context.EffectId);

    EntityId target = nodeComp->TargetId;

    // TODO (jfarris): validate the effect
    // if (FeatureEffects::ValidateEffectNode())
    {
        Value baseAmount = effectDamage.Amount().GetValue(lds);
        Value bonus = 0;

        // Apply tag bonuses
        effectDamage.TagBonuses().ForEachResolvedReadItemObject(lds, [&](uint32, const Data::TagBonus& tagBonus)
        {
            if (FeatureECS::HasTag(world, target, tagBonus.Tag.GetObjectId()))
            {
                if (HasAnyFlags(tagBonus.Flags, Data::ETagBonusFlags::AmountIsScalar))
                {
                    bonus += baseAmount * tagBonus.Amount;
                }
                else
                {
                    bonus += tagBonus.Amount;
                }
            }
        });

        // Apply damage tags
        effectDamage.DamageTags().ForEachResolvedItemObject(lds, [&](uint32, const Data::TagPtr& tag)
        {
            FeatureECS::AddTag(world, nodeId, tag.GetObjectId());
        });

        // Copy weapon tag from scope
        EffectScopeId scopeId = FeatureEffects::GetEffectScope(world, nodeId); 
        if (FeatureECS::HasTag(world, scopeId, "Weapon"_n))
        {
            FeatureECS::AddTag(world, nodeId, "Weapon"_n);
        }

        FeatureEffects::DeferEffectExecution(world, nodeId);
    }

    FeatureEffects::DereferenceEffectNode(world, nodeId);

    return true;
}

bool EffectDamageHandler::CanExecute(WorldConstRef world, const EffectContext& context) const
{
    return true;
}
