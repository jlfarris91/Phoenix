
#include "PhoenixRTS/Abilities/Ability.h"

#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixRTS/Abilities/FeatureAbilities.h"
#include "PhoenixRTS/Units/UnitId.h"

using namespace Phoenix;
using namespace Phoenix::RTS;

uint32 AbilityPriority::All()
{
    return (uint32)-1;
}

uint32 AbilityPriority::SelfTargetOrAll(const UnitId& unit, ECS::EntityId target)
{
    if (unit == target)
    {
        return (uint32)-2;
    }
    return All();
}

uint32 AbilityPriority::Closest(WorldConstRef world, const UnitId& unit, const Vec2& target)
{
    const Transform2D* transformPtr = ECS::FeatureECS::GetWorldTransformPtr(world, unit);
    if (!transformPtr)
    {
        return 0;
    }

    return (uint32)-2 - Vec2::Distance(transformPtr->Position, target).Value;
}

void IAbilityHandler::Initialize(SessionRef session)
{
}

void IAbilityHandler::Shutdown(SessionRef session)
{
}

void IAbilityHandler::OnWorldInitialize(WorldRef world)
{
}

void IAbilityHandler::OnWorldShutdown(WorldRef world)
{
}

bool IAbilityHandler::AddAbility(WorldRef world, const UnitId& unit) const
{
    return false;
}

bool IAbilityHandler::RemoveAbility(WorldRef world, const UnitId& unit) const
{
    return false;
}

bool IAbilityHandler::HasAbility(WorldConstRef world, const UnitId& unit) const
{
    return false;
}

bool IAbilityHandler::IgnoreCommand(
    WorldConstRef world,
    const AbilityCommandContext& context,
    const Command& command) const
{
    return false;
}

uint32 IAbilityHandler::GetCommandPriority(
    WorldConstRef world,
    const AbilityCommandContext& context,
    const Command& command) const
{
    return 0;
}

uint32 IAbilityHandler::GetSmartCommandPriority(
    WorldConstRef world,
    const AbilityCommandContext& context,
    const Command& command) const
{
    return 0;
}

bool IAbilityHandler::IsTransient(WorldConstRef world, const FName& abilityId) const
{
    return false;
}

bool IAbilityHandler::ExecuteOrder(WorldRef world, const UnitId& unit, const Order& order) const
{
    return false;
}

bool IAbilityHandler::InterruptOrder(WorldRef world, const UnitId& unit, const Order& order) const
{
    return false;
}

uint32 IAbilityHandler::Acquire(const Order& order) const
{
    return 0;
}

bool IAbilityHandler::SupportsMagicBox(const Order& order) const
{
    return false;
}

AbilityHandlerBase::AbilityHandlerBase(const FName& abilityId)
    : AbilityId(abilityId)
{
}

void AbilityHandlerBase::Initialize(SessionRef session)
{
    IAbilityHandler::Initialize(session);
    Abilities = session.GetFeature<FeatureAbilities>();
}

void AbilityHandlerBase::Shutdown(SessionRef session)
{
    IAbilityHandler::Shutdown(session);
    Abilities.reset();
}

FName AbilityHandlerBase::GetAbilityId() const
{
    return AbilityId;
}
