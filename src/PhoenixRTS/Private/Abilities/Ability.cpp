
#include "Abilities/Ability.h"

#include "FeatureECS.h"
#include "Abilities/FeatureAbilities.h"

using namespace Phoenix;
using namespace Phoenix::RTS;

uint32 AbilityPriority::All()
{
    return (uint32)-1;
}

uint32 AbilityPriority::SelfTargetOrAll(UnitId unit, ECS::EntityId target)
{
    if (unit == target)
    {
        return (uint32)-2;
    }
    return All();
}

uint32 AbilityPriority::Closest(WorldConstRef world, UnitId unit, const Vec2& target)
{
    const Transform2D* transformPtr = ECS::FeatureECS::GetWorldTransformPtr(world, unit);
    if (!transformPtr)
    {
        return 0;
    }

    return (uint32)-2 - Vec2::Distance(transformPtr->Position, target).Value;
}

void IAbility::Initialize(SessionRef session)
{
}

void IAbility::Shutdown(SessionRef session)
{
}

void IAbility::OnWorldInitialize(WorldRef world)
{
}

void IAbility::OnWorldShutdown(WorldRef world)
{
}

bool IAbility::AddAbility(WorldRef world, const UnitId& unit) const
{
    return false;
}

bool IAbility::RemoveAbility(WorldRef world, const UnitId& unit) const
{
    return false;
}

bool IAbility::HasAbility(WorldConstRef world, const UnitId& unit) const
{
    return false;
}

uint32 IAbility::GetCommandPriority(WorldRef world, UnitId unit, const Command& command) const
{
    return 0;
}

bool IAbility::IsTransient(WorldRef world, const FName& abilityId) const
{
    return false;
}

bool IAbility::ExecuteOrder(WorldRef world, UnitId unit, const Order& order) const
{
    
    return false;
}

bool IAbility::InterruptOrder(WorldRef world, UnitId unit, const Order& order) const
{
    return false;
}

uint32 IAbility::Acquire(const Order& order) const
{
    return 0;
}

bool IAbility::SupportsMagicBox(const Order& order) const
{
    return false;
}

AbilityBase::AbilityBase(const FName& abilityId)
    : AbilityId(abilityId)
{
}

void AbilityBase::Initialize(SessionRef session)
{
    IAbility::Initialize(session);
    Abilities = session.GetFeature<FeatureAbilities>();
}

void AbilityBase::Shutdown(SessionRef session)
{
    IAbility::Shutdown(session);
    Abilities.reset();
}

FName AbilityBase::GetAbilityId() const
{
    return AbilityId;
}
