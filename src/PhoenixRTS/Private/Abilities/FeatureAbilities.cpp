#include "Abilities/FeatureAbilities.h"

#include "FeatureLDS.h"
#include "Abilities/Ability.h"
#include "Data/DataUnit.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

bool FeatureAbilities::AddAbility(WorldRef world, const Unit& unit, const FName& abilityId)
{
}

bool FeatureAbilities::RemoveAbility(WorldRef world, const Unit& unit, const FName& abilityId)
{
}

bool FeatureAbilities::HasAbility(WorldConstRef world, const Unit& unit, const FName& abilityId)
{
}

bool FeatureAbilities::AddAbilitiesFromData(WorldRef world, const Unit& unit, const FName& unitData)
{
    const LDS::ILDSQueryContext& queryContext = *LDS::FeatureLDS::StaticGetWorldQueryContext(world);

    Data::UnitPtr unitPtr(unitData);
    (void)unitPtr.Commands.ForEachItem(queryContext, [&](const Data::CommandPtr& command)
    {
        AddAbility(world, unit, command.Ability.GetReferenceId(queryContext));
    });
}

bool FeatureAbilities::RegisterAbility(const TSharedPtr<IAbility>& ability)
{
    if (Abilities.contains(ability->GetAbilityId()))
    {
        return false;
    }

    Abilities.emplace(ability->GetAbilityId(), ability);
    return true;
}

void FeatureAbilities::OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    IFeature::OnPostWorldUpdate(world, args);
}

void FeatureAbilities::SortAbilities(WorldRef world)
{
}
