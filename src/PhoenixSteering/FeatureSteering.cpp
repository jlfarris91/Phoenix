
#include "PhoenixSteering/FeatureSteering.h"

#include "PhoenixSim/Flags.h"
#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSteering/SteeringComponent.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::Steering;

bool FeatureSteering::MoveToLocation(WorldRef world, const EntityId& entity, const Vec2& target)
{
    SteeringComponent* steerComp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    if (!steerComp)
    {
        return false;
    }

    steerComp->Mode = ESteerMode::Move;
    steerComp->GoalPos = target;
    steerComp->GoalEntity = EntityId::Invalid;
    SetFlagRef(steerComp->Flags, ESteerFlags::SeekingGoal, true);
    return true;
}

bool FeatureSteering::FollowEntity(WorldRef world, const EntityId& entity, const EntityId& target)
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
    SetFlagRef(steerComp->Flags, ESteerFlags::SeekingGoal, true);
    return true;
}

bool FeatureSteering::IsMoving(WorldConstRef world, const EntityId& entity)
{
    const SteeringComponent* steerComp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    if (!steerComp)
    {
        return false;
    }

    return steerComp->Mode == ESteerMode::Move && HasAnyFlags(steerComp->Flags, ESteerFlags::SeekingGoal);
}

bool FeatureSteering::HasFinishedMoving(WorldConstRef world, const EntityId& entity)
{
    const SteeringComponent* steerComp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    if (!steerComp)
    {
        return false;
    }

    return steerComp->Mode == ESteerMode::Move && HasAnyFlags(steerComp->Flags, ESteerFlags::ArrivedAtGoal);
}

bool FeatureSteering::TurnToFace(WorldRef world, const EntityId& entity, const EntityId& target)
{
    SteeringComponent* steerComp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    if (!steerComp)
    {
        return false;
    }

    steerComp->Mode = ESteerMode::Turn;
    steerComp->GoalPos = Vec2::Zero;
    steerComp->GoalEntity = target;
    SetFlagRef(steerComp->Flags, ESteerFlags::SeekingGoal, true);
    return true;
}

bool FeatureSteering::TurnToFace(WorldRef world, const EntityId& entity, const Vec2& target)
{
    SteeringComponent* steerComp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    if (!steerComp)
    {
        return false;
    }

    steerComp->Mode = ESteerMode::Turn;
    steerComp->GoalPos = target;
    steerComp->GoalEntity = EntityId::Invalid;
    SetFlagRef(steerComp->Flags, ESteerFlags::SeekingGoal, true);
    return true;
}

bool FeatureSteering::IsTurning(WorldConstRef world, const EntityId& entity)
{
    const SteeringComponent* steerComp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    if (!steerComp)
    {
        return false;
    }

    return steerComp->Mode == ESteerMode::Turn && HasAnyFlags(steerComp->Flags, ESteerFlags::SeekingGoal);
}

bool FeatureSteering::HasFinishedTurning(WorldConstRef world, const EntityId& entity)
{
    const SteeringComponent* steerComp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    if (!steerComp)
    {
        return false;
    }

    return steerComp->Mode == ESteerMode::Turn && HasAnyFlags(steerComp->Flags, ESteerFlags::ArrivedAtGoal);
}

TOptional<ESteerMode> FeatureSteering::GetSteeringMode(WorldRef world, const EntityId& entity)
{
    SteeringComponent* steerComp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    return steerComp ? steerComp->Mode : TOptional<ESteerMode>();
}

bool FeatureSteering::IsSeekingGoal(WorldRef world, const EntityId& entity)
{
    SteeringComponent* steerComp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    if (!steerComp)
    {
        return false;
    }

    return HasAnyFlags(steerComp->Flags, ESteerFlags::SeekingGoal);
}

bool FeatureSteering::HasArrivedAtGoal(WorldRef world, const EntityId& entity)
{
    SteeringComponent* steerComp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    if (!steerComp)
    {
        return false;
    }

    return HasAnyFlags(steerComp->Flags, ESteerFlags::ArrivedAtGoal);
}

bool FeatureSteering::Stop(WorldRef world, const EntityId& entity)
{
    SteeringComponent* steerComp = FeatureECS::GetComponent<SteeringComponent>(world, entity);
    if (!steerComp)
    {
        return false;
    }

    steerComp->Mode = ESteerMode::Idle;
    steerComp->GoalEntity = EntityId::Invalid;
    steerComp->GoalPos = Vec2::Zero;
    SetFlagRef(steerComp->Flags, ESteerFlags::SeekingGoal, false);
    SetFlagRef(steerComp->Flags, ESteerFlags::ArrivedAtGoal, false);
    return true;
}

bool FeatureSteering::UpdateSpeed(WorldRef world, const EntityId& entity, const SteeringSpeedArgs& args)
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

void FeatureSteering::Initialize()
{
    IFeature::Initialize();

    SteeringSystem = MakeShared<Steering::SteeringSystem>();

    TSharedPtr<FeatureECS> featureECS = Session->GetFeatureSet()->GetFeature<FeatureECS>();
    featureECS->RegisterSystem(SteeringSystem);
}

void FeatureSteering::Shutdown()
{
    IFeature::Shutdown();

    TSharedPtr<FeatureECS> featureECS = Session->GetFeatureSet()->GetFeature<FeatureECS>();
    featureECS->UnregisterSystem(SteeringSystem);
}

bool FeatureSteering::OnHandleWorldAction(WorldRef world, const FeatureActionArgs& action)
{
    return IFeature::OnHandleWorldAction(world, action);
}
