#include "Phoenix.Sim.RTS/Abilities/FeatureAbilities.h"

#include "Phoenix.Sim.ECS/FeatureECS.h"
#include "Phoenix.Sim.LDS/FeatureLDS.h"

#include "Phoenix.Sim.RTS/Abilities/AbilityHandler.h"
#include "Phoenix.Sim.RTS/Data/DataUnit.h"
#include "Phoenix.Sim.RTS/Units/FeatureUnit.h"
#include "Phoenix.Sim.RTS/Units/UnitId.h"
#include "Phoenix.Sim/Session.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

void FeatureAbilities::RegisterAbilityHandler(const std::shared_ptr<IAbilityHandler>& handler)
{
    AbilityIdToHandlerMap.emplace(handler->GetCommandId(), handler);
}

bool FeatureAbilities::UnregisterAbilityHandler(const FName& abilityId)
{
    return AbilityIdToHandlerMap.erase(abilityId) > 0;
}

std::shared_ptr<IAbilityHandler> FeatureAbilities::FindAbilityHandlerCached(WorldConstRef world, const FName& abilityId)
{
    std::shared_ptr<IAbilityHandler> handler = FindAbilityHandler(world, abilityId);

    // Cache the handler for this ability id for faster subsequent lookups even if it is null.
    AbilityIdToHandlerMap.emplace(abilityId, handler);

    return handler;
}

std::shared_ptr<IAbilityHandler> FeatureAbilities::FindAbilityHandler(WorldConstRef world, const FName& abilityId) const
{
    std::shared_ptr<const ILDSQueryContext> queryContext = FeatureLDS::StaticGetWorldQueryContext(world);
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

std::shared_ptr<IAbilityHandler> FeatureAbilities::StaticFindAbilityHandler(WorldConstRef world, const FName& abilityId)
{
    std::shared_ptr<FeatureAbilities> feature = GetFeature<FeatureAbilities>(world);
    return feature ? feature->FindAbilityHandlerCached(world, abilityId) : std::shared_ptr<IAbilityHandler>{};
}

bool FeatureAbilities::AddAbility(WorldRef world, const UnitId& unit, const FName& abilityId)
{
    std::shared_ptr<IAbilityHandler> ability = StaticFindAbilityHandler(world, abilityId);
    if (!ability)
    {
        return false;
    }

    return ability->AddAbility(world, unit);
}

bool FeatureAbilities::RemoveAbility(WorldRef world, const UnitId& unit, const FName& abilityId)
{
    std::shared_ptr<IAbilityHandler> ability = StaticFindAbilityHandler(world, abilityId);
    if (!ability)
    {
        return false;
    }

    return ability->RemoveAbility(world, unit);
}

bool FeatureAbilities::HasAbility(WorldConstRef world, const UnitId& unit, const FName& abilityId)
{
    std::shared_ptr<IAbilityHandler> ability = StaticFindAbilityHandler(world, abilityId);
    if (!ability)
    {
        return false;
    }

    return ability->HasAbility(world, unit);
}

bool FeatureAbilities::AddAbilitiesFromData(WorldRef world, const UnitId& unit, const FName& unitData)
{
    bool success = true;

    std::vector<FName> abilities;
    GetAbilities(world, unit, abilities);

    for (const FName& abilityId : abilities)
    {
        success = AddAbility(world, unit, abilityId) && success;
        // PHX_ASSERT(success);
    }

    return success;
}

uint32 FeatureAbilities::GetAbilities(WorldConstRef world, const UnitId& unit, std::vector<FName>& outAbilityIds)
{
    const ILDSQueryContext& queryContext = *FeatureLDS::StaticGetWorldQueryContext(world);

    Data::UnitPtr unitPtr(FeatureUnit::GetUnitDataId(world, unit));
    (void)unitPtr.Commands().ForEachItem(queryContext, [&](uint32, const Data::CommandPtr& command)
    {
        FName abilityId = command.Ability().GetReferenceId(queryContext);

        if (std::ranges::find(outAbilityIds, abilityId) != outAbilityIds.end())
        {
            return;
        }

        if (!FName::IsNoneOrEmpty(abilityId))
        {
            outAbilityIds.push_back(abilityId);
        }
    });

    return static_cast<uint32>(outAbilityIds.size());
}

void FeatureAbilities::Initialize(const std::shared_ptr<Phoenix::Session>& session)
{
    IFeature::Initialize(session);

    std::vector<std::shared_ptr<IAbilityHandler>> handlers;
    Session->GetServices2<IAbilityHandler>(handlers);

    for (const std::shared_ptr<IAbilityHandler>& handler : handlers)
    {
        RegisterAbilityHandler(handler);
    }

    EntityReleasingHandle = session->GetFeature<FeatureECS>()->OnEntityReleasing().AddStatic(&FeatureAbilities::OnEntityReleasing);
}

void FeatureAbilities::Shutdown()
{
    while (!AbilityIdToHandlerMap.empty())
    {
        UnregisterAbilityHandler(AbilityIdToHandlerMap.begin()->first);
    }

    Session->GetFeature<FeatureECS>()->OnEntityReleasing().Remove(EntityReleasingHandle);
    EntityReleasingHandle = {};

    IFeature::Shutdown();
}

void FeatureAbilities::OnEntityReleasing(WorldRef world, EntityId entity)
{
    RemoveAllAbilities(world, entity);
}

void FeatureAbilities::RemoveAllAbilities(WorldRef world, EntityId entity)
{
    std::vector<FName> abilities;
    if (GetAbilities(world, UnitId(entity), abilities) == 0)
    {
        return;
    }

    for (FName abilityId : abilities)
    {
        RemoveAbility(world, UnitId(entity), abilityId);
    }
}
