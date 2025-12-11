
#include "FeatureSteering.h"

#include "FeatureECS.h"
#include "Flags.h"
#include "SteeringComponent.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::Steering;

bool FeatureSteering::MoveToLocation(WorldRef world, const EntityId& entity, const Vec2& target)
{
    SeekComponent* seekComp = FeatureECS::GetComponent<SeekComponent>(world, entity);
    if (!seekComp)
    {
        return false;
    }

    seekComp->TargetPos = target;
    seekComp->TargetEntity = EntityId::Invalid;
    SetFlagRef(seekComp->Flags, ESeekFlags::SeekingGoal, true);
    return true;
}

bool FeatureSteering::FollowEntity(WorldRef world, const EntityId& entity, const EntityId& target)
{
    if (!FeatureECS::IsEntityValid(world, target))
    {
        return false;
    }

    SeekComponent* seekComp = FeatureECS::GetComponent<SeekComponent>(world, entity);
    if (!seekComp)
    {
        return false;
    }

    seekComp->TargetEntity = target;
    seekComp->TargetPos = Vec2::Zero;
    SetFlagRef(seekComp->Flags, ESeekFlags::SeekingGoal, true);
    return true;
}

bool FeatureSteering::IsSeekingGoal(WorldRef world, const EntityId& entity)
{
    SeekComponent* seekComp = FeatureECS::GetComponent<SeekComponent>(world, entity);
    if (!seekComp)
    {
        return false;
    }

    return HasAnyFlags(seekComp->Flags, ESeekFlags::SeekingGoal);
}

bool FeatureSteering::HasArrivedAtGoal(WorldRef world, const EntityId& entity)
{
    SeekComponent* seekComp = FeatureECS::GetComponent<SeekComponent>(world, entity);
    if (!seekComp)
    {
        return false;
    }

    return HasAnyFlags(seekComp->Flags, ESeekFlags::ArrivedAtGoal);
}

bool FeatureSteering::Stop(WorldRef world, const EntityId& entity)
{
    SeekComponent* seekComp = FeatureECS::GetComponent<SeekComponent>(world, entity);
    if (!seekComp)
    {
        return false;
    }

    seekComp->TargetEntity = EntityId::Invalid;
    seekComp->TargetPos = Vec2::Zero;
    SetFlagRef(seekComp->Flags, ESeekFlags::SeekingGoal, false);
    SetFlagRef(seekComp->Flags, ESeekFlags::ArrivedAtGoal, false);
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
