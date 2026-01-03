#include "PhoenixRTS/Abilities/FeatureAbilities.h"

#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/LDS/FeatureLDS.h"

#include "PhoenixRTS/Abilities/AbilityHandler.h"
#include "PhoenixRTS/Data/DataUnit.h"
#include "PhoenixRTS/Units/FeatureUnit.h"
#include "PhoenixRTS/Units/UnitId.h"
#include "PhoenixSim/Session.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

FeatureAbilities::FeatureAbilities()
{
    FEATURE_CHANNEL(FeatureChannels::HandleWorldAction)
    FEATURE_CHANNEL(FeatureChannels::WorldInitialize)
    FEATURE_CHANNEL(FeatureChannels::WorldShutdown)
}

void FeatureAbilities::RegisterAbilityHandler(const TSharedPtr<IAbilityHandler>& handler)
{
    AbilityIdToHandlerMap.emplace(handler->GetCommandId(), handler);
}

bool FeatureAbilities::UnregisterAbilityHandler(const FName& abilityId)
{
    return AbilityIdToHandlerMap.erase(abilityId) > 0;
}

TSharedPtr<IAbilityHandler> FeatureAbilities::FindAbilityHandlerCached(WorldConstRef world, const FName& abilityId)
{
    TSharedPtr<IAbilityHandler> handler = FindAbilityHandler(world, abilityId);

    // Cache the handler for this ability id for faster subsequent lookups even if it is null.
    AbilityIdToHandlerMap.emplace(abilityId, handler);

    return handler;
}

TSharedPtr<IAbilityHandler> FeatureAbilities::FindAbilityHandler(WorldConstRef world, const FName& abilityId) const
{
    TSharedPtr<const ILDSQueryContext> queryContext = FeatureLDS::StaticGetWorldQueryContext(world);
    if (!queryContext)
    {
        return {};
    }

    FName currentObjectId = abilityId;
    for (;;)
    {
        auto effectHandlerIter = AbilityIdToHandlerMap.find(currentObjectId);
        if (effectHandlerIter != AbilityIdToHandlerMap.end())
        {
            return effectHandlerIter->second;
        }

        // Find the base object id
        const LDSRecord* baseRecord = queryContext->QueryRecord({ abilityId, "/base"_n }, ELDSRecordQueryFlags::Exact);
        if (!baseRecord)
        {
            return {};
        }

        FName baseId = baseRecord->GetValueAs<FName>();
        if (baseId == currentObjectId)
        {
            return {};
        }

        currentObjectId = baseId;
    }
}

TSharedPtr<IAbilityHandler> FeatureAbilities::StaticFindAbilityHandler(WorldConstRef world, const FName& abilityId)
{
    TSharedPtr<FeatureAbilities> feature = GetFeature<FeatureAbilities>(world);
    return feature ? feature->FindAbilityHandlerCached(world, abilityId) : TSharedPtr<IAbilityHandler>{};
}

bool FeatureAbilities::AddAbility(WorldRef world, const UnitId& unit, const FName& abilityId)
{
    TSharedPtr<IAbilityHandler> ability = StaticFindAbilityHandler(world, abilityId);
    if (!ability)
    {
        return false;
    }

    return ability->AddAbility(world, unit);
}

bool FeatureAbilities::RemoveAbility(WorldRef world, const UnitId& unit, const FName& abilityId)
{
    TSharedPtr<IAbilityHandler> ability = StaticFindAbilityHandler(world, abilityId);
    if (!ability)
    {
        return false;
    }

    return ability->RemoveAbility(world, unit);
}

bool FeatureAbilities::HasAbility(WorldConstRef world, const UnitId& unit, const FName& abilityId)
{
    TSharedPtr<IAbilityHandler> ability = StaticFindAbilityHandler(world, abilityId);
    if (!ability)
    {
        return false;
    }

    return ability->HasAbility(world, unit);
}

bool FeatureAbilities::AddAbilitiesFromData(WorldRef world, const UnitId& unit, const FName& unitData)
{
    bool success = true;

    TArray2<FName> abilities;
    GetAbilities(world, unit, abilities);

    for (const FName& abilityId : abilities)
    {
        success = AddAbility(world, unit, abilityId) && success;
        // PHX_ASSERT(success);
    }

    return success;
}

uint32 FeatureAbilities::GetAbilities(WorldConstRef world, const UnitId& unit, TArray2<FName>& outAbilityIds)
{
    const ILDSQueryContext& queryContext = *FeatureLDS::StaticGetWorldQueryContext(world);

    Data::UnitPtr unitPtr(FeatureUnit::GetUnitDataId(world, unit));
    (void)unitPtr.Commands().ForEachItem(queryContext, [&](uint32, const Data::CommandPtr& command)
    {
        FName abilityId = command.Ability().GetReferenceId(queryContext);
        if (!FName::IsNoneOrEmpty(abilityId))
        {
            outAbilityIds.PushBackUnique(abilityId);
        }
    });

    return static_cast<uint32>(outAbilityIds.Num());
}

void FeatureAbilities::Initialize(const TSharedPtr<Phoenix::Session>& session)
{
    IFeature::Initialize(session);

    TArray2<TSharedPtr<IAbilityHandler>> handlers;
    Session->GetServices2<IAbilityHandler>(handlers);

    for (const TSharedPtr<IAbilityHandler>& handler : handlers)
    {
        RegisterAbilityHandler(handler);
    }
}

void FeatureAbilities::Shutdown()
{
    IFeature::Shutdown();

    while (!AbilityIdToHandlerMap.empty())
    {
        UnregisterAbilityHandler(AbilityIdToHandlerMap.begin()->first);
    }
}
