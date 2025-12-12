
#include "FeatureSteering.h"

#include "FeatureECS.h"
#include "Flags.h"
#include "SteeringComponent.h"

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

    steerComp->GoalEntity = target;
    steerComp->GoalPos = Vec2::Zero;
    SetFlagRef(steerComp->Flags, ESteerFlags::SeekingGoal, true);
    return true;
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
