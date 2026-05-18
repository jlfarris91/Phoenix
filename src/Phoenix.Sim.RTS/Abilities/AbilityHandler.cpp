
#include "Phoenix.Sim.RTS/Abilities/AbilityHandler.h"

#include "Phoenix.Sim.ECS/FeatureECS.h"
#include "Phoenix.Sim.RTS/Abilities/FeatureAbilities.h"
#include "Phoenix.Sim.RTS/Units/UnitId.h"
#include "Phoenix.Sim/Session.h"

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

void IAbilityHandler::Initialize(const std::shared_ptr<Phoenix::Session>& session)
{
    ICommandHandler::Initialize(session);
    AbilitiesFeature = session->GetFeature<FeatureAbilities>();
}

void IAbilityHandler::Shutdown()
{
    AbilitiesFeature.reset();
    ICommandHandler::Shutdown();
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
