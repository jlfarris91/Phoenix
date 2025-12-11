#include "Abilities/FeatureAbilities.h"

#include "FeatureLDS.h"
#include "Abilities/Ability.h"
#include "Data/DataUnit.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

bool FeatureAbilities::RegisterAbility(const TSharedPtr<IAbility>& ability)
{
    if (Abilities.contains(ability->GetAbilityId()))
    {
        return false;
    }

    Abilities.emplace(ability->GetAbilityId(), ability);
    return true;
}

TSharedPtr<IAbility> FeatureAbilities::GetAbility(const FName& abilityId) const
{
    auto iter = Abilities.find(abilityId);
    return iter != Abilities.end() ? iter->second : TSharedPtr<IAbility>{};
}

TSharedPtr<IAbility> FeatureAbilities::StaticGetAbility(WorldConstRef world, const FName& abilityId)
{
    TSharedPtr<FeatureAbilities> feature = GetFeature<FeatureAbilities>(world);
    return feature ? feature->GetAbility(abilityId) : TSharedPtr<IAbility>{};
}

bool FeatureAbilities::AddAbility(WorldRef world, const UnitId& unit, const FName& abilityId)
{
    TSharedPtr<IAbility> ability = StaticGetAbility(world, abilityId);
    if (!ability)
    {
        return false;
    }

    return ability->AddAbility(world, unit);
}

bool FeatureAbilities::RemoveAbility(WorldRef world, const UnitId& unit, const FName& abilityId)
{
    TSharedPtr<IAbility> ability = StaticGetAbility(world, abilityId);
    if (!ability)
    {
        return false;
    }

    return ability->RemoveAbility(world, unit);
}

bool FeatureAbilities::HasAbility(WorldConstRef world, const UnitId& unit, const FName& abilityId)
{
    TSharedPtr<IAbility> ability = StaticGetAbility(world, abilityId);
    if (!ability)
    {
        return false;
    }

    return ability->HasAbility(world, unit);
}

bool FeatureAbilities::AddAbilitiesFromData(WorldRef world, const UnitId& unit, const FName& unitData)
{
    const LDS::ILDSQueryContext& queryContext = *LDS::FeatureLDS::StaticGetWorldQueryContext(world);

    bool success = true;

    Data::UnitPtr unitPtr(unitData);
    (void)unitPtr.Commands.ForEachItem(queryContext, [&](uint32, const Data::CommandPtr& command)
    {
        FName abilityId = command.Ability.GetReferenceId(queryContext);
        if (!FName::IsNoneOrEmpty(abilityId))
        {
            success = AddAbility(world, unit, abilityId) && success;
            PHX_ASSERT(success);
        }
    });

    return success;
}

void FeatureAbilities::Initialize()
{
    IFeature::Initialize();

    for (const TSharedPtr<IAbility>& ability : Abilities | std::views::values)
    {
        ability->Initialize(*Session);
    }
}

void FeatureAbilities::Shutdown()
{
    IFeature::Shutdown();

    for (const TSharedPtr<IAbility>& ability : Abilities | std::views::values)
    {
        ability->Shutdown(*Session);
    }
}

void FeatureAbilities::OnWorldInitialize(WorldRef world)
{
    IFeature::OnWorldInitialize(world);

    for (const TSharedPtr<IAbility>& ability : Abilities | std::views::values)
    {
        ability->OnWorldInitialize(world);
    }
}

void FeatureAbilities::OnWorldShutdown(WorldRef world)
{
    IFeature::OnWorldShutdown(world);

    for (const TSharedPtr<IAbility>& ability : Abilities | std::views::values)
    {
        ability->OnWorldShutdown(world);
    }
}
