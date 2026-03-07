
#include "PhoenixRTS/Abilities/AbilityHandler.h"

#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixRTS/Abilities/FeatureAbilities.h"
#include "PhoenixRTS/Units/UnitId.h"
#include "PhoenixSim/Session.h"

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

void IAbilityHandler::Initialize(const TSharedPtr<Phoenix::Session>& session)
{
    ICommandHandler::Initialize(session);
    AbilitiesFeature = session->GetFeature<FeatureAbilities>();
}

void IAbilityHandler::Shutdown()
{
    ICommandHandler::Shutdown();
    AbilitiesFeature.reset();
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

ECS::EntityId IAbilityHandler::ScanForTarget(WorldConstRef world, const AbilityTargetScanArgs& args) const
{
    return ECS::EntityId::Invalid;
}
