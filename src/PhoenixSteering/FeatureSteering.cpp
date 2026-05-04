#include "PhoenixSteering/FeatureSteering.h"

#include "PhoenixSim/Flags.h"
#include "PhoenixSim/MortonCode.h"
#include "PhoenixSim/Session.h"
#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSteering/SteeringComponent.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::Steering;

bool FeatureSteering::CanMove(WorldConstRef world, EntityId entity)
{
    const SteeringComponent* steerComp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    return steerComp && steerComp->MaxSpeed > 0 && steerComp->Mode != ESteerMode::Hold;
}

bool FeatureSteering::CanTurn(WorldConstRef world, EntityId entity)
{
    const SteeringComponent* steerComp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    return steerComp && (steerComp->TurnRateIdle > 0 || steerComp->TurnRateMoving > 0);
}

bool FeatureSteering::MoveToLocation(WorldRef world, EntityId entity, const Vec2& target, Distance range)
{
    SteeringComponent* steerComp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    if (!steerComp)
    {
        return false;
    }

    steerComp->Mode = ESteerMode::Move;
    steerComp->GoalPos = target;
    steerComp->GoalEntity = EntityId::Invalid;
    steerComp->ArrivalRange = range;
    steerComp->Slack = 0;
    steerComp->SpreadingSlack = 0;
    steerComp->BestPos = Vec2::Max;
    steerComp->PreviousPos = FeatureECS::GetWorldPosition(world, entity);
    SetFlagRef(steerComp->Flags, ESteerFlags::SeekingGoal, true);
    ClearFlagRef(steerComp->Flags, ESteerFlags::SteeringLeft);
    ClearFlagRef(steerComp->Flags, ESteerFlags::SteeringRight);
    return true;
}

bool FeatureSteering::FollowEntity(WorldRef world, EntityId entity, EntityId target, Distance range)
{
    if (!FeatureECS::IsEntityValid(world, target))
    {
        return false;
    }

    SteeringComponent* steerComp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    if (!steerComp)
    {
        return false;
    }

    steerComp->Mode = ESteerMode::Move;
    steerComp->GoalEntity = target;
    steerComp->GoalPos = Vec2::Zero;
    steerComp->ArrivalRange = range;
    steerComp->Slack = 0;
    steerComp->SpreadingSlack = 0;
    steerComp->BestPos = Vec2::Max;
    steerComp->PreviousPos = FeatureECS::GetWorldPosition(world, entity);
    SetFlagRef(steerComp->Flags, ESteerFlags::SeekingGoal, true);
    ClearFlagRef(steerComp->Flags, ESteerFlags::SteeringLeft);
    ClearFlagRef(steerComp->Flags, ESteerFlags::SteeringRight);
    return true;
}

bool FeatureSteering::IsMoving(WorldConstRef world, EntityId entity)
{
    const SteeringComponent* steerComp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    if (!steerComp)
    {
        return false;
    }

    return steerComp->Mode == ESteerMode::Move && HasAnyFlags(steerComp->Flags, ESteerFlags::SeekingGoal);
}

bool FeatureSteering::HasFinishedMoving(WorldConstRef world, EntityId entity)
{
    const SteeringComponent* steerComp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    if (!steerComp)
    {
        return false;
    }

    return steerComp->Mode == ESteerMode::Move && HasAnyFlags(steerComp->Flags, ESteerFlags::ArrivedAtGoal);
}

bool FeatureSteering::TurnToFace(WorldRef world, EntityId entity, EntityId target)
{
    SteeringComponent* steerComp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    if (!steerComp)
    {
        return false;
    }

    steerComp->Mode = ESteerMode::Turn;
    steerComp->GoalPos = Vec2::Zero;
    steerComp->GoalEntity = target;
    steerComp->Slack = 0;
    steerComp->SpreadingSlack = 0;
    steerComp->BestPos = Vec2::Max;
    SetFlagRef(steerComp->Flags, ESteerFlags::SeekingGoal, true);
    return true;
}

bool FeatureSteering::TurnToFace(WorldRef world, EntityId entity, const Vec2& target)
{
    SteeringComponent* steerComp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    if (!steerComp)
    {
        return false;
    }

    steerComp->Mode = ESteerMode::Turn;
    steerComp->GoalPos = target;
    steerComp->GoalEntity = EntityId::Invalid;
    steerComp->Slack = 0;
    steerComp->SpreadingSlack = 0;
    steerComp->BestPos = Vec2::Max;
    SetFlagRef(steerComp->Flags, ESteerFlags::SeekingGoal, true);
    return true;
}

bool FeatureSteering::IsTurning(WorldConstRef world, EntityId entity)
{
    const SteeringComponent* steerComp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    if (!steerComp)
    {
        return false;
    }

    return steerComp->Mode == ESteerMode::Turn && HasAnyFlags(steerComp->Flags, ESteerFlags::SeekingGoal);
}

bool FeatureSteering::HasFinishedTurning(WorldConstRef world, EntityId entity)
{
    const SteeringComponent* steerComp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    if (!steerComp)
    {
        return false;
    }

    return steerComp->Mode == ESteerMode::Turn && HasAnyFlags(steerComp->Flags, ESteerFlags::ArrivedAtGoal);
}

TOptional<ESteerMode> FeatureSteering::GetSteeringMode(WorldConstRef world, EntityId entity)
{
    const SteeringComponent* steerComp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    return steerComp ? steerComp->Mode : TOptional<ESteerMode>();
}

bool FeatureSteering::IsSeekingGoal(WorldConstRef world, EntityId entity)
{
    const SteeringComponent* steerComp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    if (!steerComp)
    {
        return false;
    }

    return HasAnyFlags(steerComp->Flags, ESteerFlags::SeekingGoal);
}

bool FeatureSteering::HasArrivedAtGoal(WorldConstRef world, EntityId entity)
{
    const SteeringComponent* steerComp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    if (!steerComp)
    {
        return false;
    }

    return HasAnyFlags(steerComp->Flags, ESteerFlags::ArrivedAtGoal);
}

bool FeatureSteering::Stop(WorldRef world, EntityId entity)
{
    SteeringComponent* steerComp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    if (!steerComp)
    {
        return false;
    }

    steerComp->Mode = ESteerMode::Idle;
    steerComp->GoalEntity = EntityId::Invalid;
    // steerComp->GoalPos = Vec2::Zero;
    steerComp->Slack = 0;
    steerComp->SpreadingSlack = 0;
    steerComp->BestPos = Vec2::Max;
    ClearFlagRef(steerComp->Flags, ESteerFlags::SeekingGoal);
    ClearFlagRef(steerComp->Flags, ESteerFlags::ArrivedAtGoal);
    ClearFlagRef(steerComp->Flags, ESteerFlags::SteeringLeft);
    ClearFlagRef(steerComp->Flags, ESteerFlags::SteeringRight);
    return true;
}

bool FeatureSteering::IsHolding(WorldConstRef world, EntityId entity)
{
    const SteeringComponent* comp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    return comp && comp->Mode == ESteerMode::Hold;
}

bool FeatureSteering::SetHolding(WorldRef world, EntityId entity, bool holding)
{
    SteeringComponent* comp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    if (!comp)
    {
        return false;
    }

    comp->Mode = ESteerMode::Hold;
    return true;
}

bool FeatureSteering::UpdateSpeed(WorldRef world, EntityId entity, const SteeringSpeedArgs& args)
{
    SteeringComponent* comp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    if (!comp)
    {
        return false;
    }

    comp->MaxSpeed = args.MaxSpeed.GetValue(comp->MaxSpeed);
    comp->AccelerationTime = args.AccelerationTime.GetValue(comp->AccelerationTime);
    comp->DecelerationTime = args.DecelerationTime.GetValue(comp->DecelerationTime);
    return true;
}

Distance FeatureSteering::GetEntityInnerRadius(WorldConstRef world, EntityId entity)
{
    const SteeringComponent* comp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    if (!comp)
    {
        return 0.0;
    }

    return comp->InnerRadius;
}

Distance FeatureSteering::GetEntityOuterRadius(WorldConstRef world, EntityId entity)
{
    const SteeringComponent* comp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    if (!comp)
    {
        return 0.0;
    }

    return comp->OuterRadius;
}

uint32 FeatureSteering::QueryEntitiesInRange(
    WorldConstRef world,
    const Vec2& pos,
    Distance range,
    std::vector<const SortedEntity*>& outEntities,
    const SteeringRangeQueryArgs& args)
{
    const FeatureSteeringScratchBlock& block = world.GetBlockRef<FeatureSteeringScratchBlock>();

    TMortonCodeRangeArray ranges;
    ranges.reserve(8);

    MortonCodeAABB aabb = ToMortonCodeAABB(pos, range + block.MaxEntityRadius);
    MortonCodeQuery(aabb, ranges);

    uint32 num = 0;
    ForEachInMortonCodeRanges<SortedEntity, &SortedEntity::ZCode>(
        block.SortedEntities,
        ranges,
        [&](const SortedEntity& entity)
        {
            if (args.Exclude.contains(entity.EntityId))
            {
                return false;
            }

            if ((entity.SteeringComponent->CollisionMask & args.CollisionMask) == 0)
            {
                return false;
            }

            outEntities.push_back(&entity);
            return ++num == args.MaxNum;
        });

    return num;
}

void FeatureSteering::Initialize(const std::shared_ptr<Phoenix::Session>& session)
{
    IFeature::Initialize(session);

    SteeringSystem = std::make_shared<Steering::SteeringSystem>();

    std::shared_ptr<FeatureECS> featureECS = Session->GetFeatureSet()->GetFeature<FeatureECS>();
    featureECS->RegisterSystem(SteeringSystem);
}

void FeatureSteering::Shutdown()
{
    std::shared_ptr<FeatureECS> featureECS = Session->GetFeatureSet()->GetFeature<FeatureECS>();
    featureECS->UnregisterSystem(SteeringSystem);
    
    IFeature::Shutdown();
}

bool FeatureSteering::OnHandleWorldAction(WorldRef world, const FeatureActionArgs& action)
{
    return IFeature::OnHandleWorldAction(world, action);
}
