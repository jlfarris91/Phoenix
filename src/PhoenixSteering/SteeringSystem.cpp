
#include "PhoenixSteering/SteeringSystem.h"
#include "PhoenixSim/Reflection/Registration.h"

#include "PhoenixSim/Debug/Debug.h"
#include "PhoenixSim/Flags.h"
#include "PhoenixSim/MortonCode.h"
#include "PhoenixSim/WorldTaskQueue.h"
#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/ECS/SystemJob.h"
#include "PhoenixSim/ECS/TransformComponent.h"
#include "PhoenixSim/Navigation/FeatureNavigation.h"
#include "PhoenixPhysics/FeaturePhysics.h"
#include "PhoenixSteering/FeatureSteering.h"
#include "PhoenixSteering/SteeringComponent.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::Physics;
using namespace Phoenix::Steering;

namespace SteeringDetail
{
    struct PopulateSortedEntitiesJob : IBufferJob<TransformComponent&, SteeringComponent&>
    {
        void Execute(const EntityComponentSpan<TransformComponent&, SteeringComponent&>& span) override
        {
            PHX_PROFILE_ZONE_SCOPED_N("PopulateSortedEntitiesJob");

            FeatureSteeringScratchBlock& scratchBlock = World->GetBlockRef<FeatureSteeringScratchBlock>();

            for (auto && [entityId, index, transformComp, steeringComp] : span)
            {
                uint32 sortedEntityIndex = scratchBlock.SortedEntityCount.fetch_add(1);
                scratchBlock.SortedEntities[sortedEntityIndex] = SortedEntity{entityId, &transformComp, &steeringComp, transformComp.ZCode};
            }
        }
    };

    void SortEntitiesByZCodeTask(WorldRef world)
    {
        PHX_PROFILE_ZONE_SCOPED;

        FeatureSteeringScratchBlock& scratchBlock = world.GetBlockRef<FeatureSteeringScratchBlock>();

        // Calculated from PopulateSortedEntitiesJob
        scratchBlock.SortedEntities.SetSize(scratchBlock.SortedEntityCount);

        // Sort entities by their zcodes
        std::ranges::stable_sort(
            scratchBlock.SortedEntities,
            [](const SortedEntity& a, const SortedEntity& b)
            {
                return a.ZCode < b.ZCode;
            });

        scratchBlock.MaxEntityRadius = 0;
        for (const SortedEntity& entity : scratchBlock.SortedEntities)
        {
            if (entity.SteeringComponent->OuterRadius > scratchBlock.MaxEntityRadius)
            {
                scratchBlock.MaxEntityRadius = entity.SteeringComponent->OuterRadius;
            }
        }
    }

    struct PathfindingJob : IBufferJob<TransformComponent&, SteeringComponent&>
    {
        Distance ArrivalThreshold;

        void Execute(const EntityComponentSpan<TransformComponent&, SteeringComponent&>& span) override
        {
            PHX_PROFILE_ZONE_SCOPED_N("PathfindingJob");

            for (auto && [
                entityId,
                index,
                transformComp,
                steerComp] : span)
            {
                DetermineStepPositions(transformComp, steerComp);
            }
        }

        void DetermineStepPositions(const TransformComponent& transformComp, SteeringComponent& steerComp) const
        {
            PHX_PROFILE_ZONE_SCOPED;

            // Only step entities actively seeking a goal
            if (!HasAnyFlags(steerComp.Flags, ESteerFlags::SeekingGoal))
            {
                return;
            }

            bool clearSeekingGoal = true;
            if (steerComp.GoalEntity != EntityId::Invalid)
            {
                if (const Transform2D* targetTransform = FeatureECS::GetWorldTransformPtr(*World, steerComp.GoalEntity))
                {
                    steerComp.GoalPos = targetTransform->Position;

                    // Only clear the seeking goal flag if within range of a static position.
                    // Moving towards an entity will continue until the target is cleared or becomes invalid.
                    clearSeekingGoal = false;
                }
                else
                {
                    // Target entity is no longer a valid target
                    steerComp.GoalEntity = EntityId::Invalid;
                    ClearFlagRef(steerComp.Flags, ESteerFlags::SeekingGoal);
                    return;
                }
            }

            Vec2 targetPos = steerComp.GoalPos;
            const Transform2D& transform = transformComp.Transform;
            const Vec2& currPos = transform.Position;

            // Pathfinding::PathResult result = Pathfinding::FeatureNavigation::PathTo(*World, currPos, targetPos, steerComp.OuterRadius);
            // if (result.PathFound)
            // {
            //     targetPos = result.NextPoint;
            // }
            // else
            // {
            //     ClearFlagRef(steerComp.Flags, ESteerFlags::SeekingGoal);
            //     ClearFlagRef(steerComp.Flags, ESteerFlags::ArrivedAtGoal);
            //     return;
            // }

            Vec2 targetOffset = targetPos - currPos;
            Distance distance = targetOffset.Length();

            Distance arrivalRange = Max(ArrivalThreshold, steerComp.ArrivalRange);

            // The entity has reached its goal
            if (distance < arrivalRange || distance < steerComp.Slack)
            {
                SetFlagRef(steerComp.Flags, ESteerFlags::ArrivedAtGoal);

                if (clearSeekingGoal)
                {
                    ClearFlagRef(steerComp.Flags, ESteerFlags::SeekingGoal);
                }
            }

            steerComp.StepPos[0] = targetPos;
        }
    };

    struct SteeringJob : IBufferJob<TransformComponent&, SteeringComponent&>
    {
        DeltaTime DeltaTime;

        bool MoveTowardsGoal;
        Distance ArrivalThreshold;

        Distance DensityScalar;
        Distance DensityRadiusScalar;
        Distance AvoidanceScalar;
        Distance AvoidanceRadiusScalar;

        Value SlackIncreaseRate;
        Value SlackIncreaseRateFast;
        Value SlackRateDivisor;
        Value SlackRateDivisorSlow;
        Value MaxSlack;

        void Execute(const EntityComponentSpan<TransformComponent&, SteeringComponent&>& span) override
        {
            PHX_PROFILE_ZONE_SCOPED_N("SteeringJob");

            for (auto && [
                entityId,
                index,
                transformComp,
                steerComp] : span)
            {
                if (!HasAnyFlags(steerComp.Flags, ESteerFlags::SeekingGoal))
                {
                    continue;
                }
                
                const Vec2& currPos = transformComp.Transform.Position;
                Vec2 vel = currPos - steerComp.PreviousPos;

                auto vecToStep0 = steerComp.StepPos[0] - currPos;
                auto distToStep0 = vecToStep0.Length();

                if (steerComp.Mode == ESteerMode::Idle)
                {
                    while (vel.Length() > steerComp.MaxSpeed)
                    {
                        vel /= 2;
                    }
                    vel = vel / 8 * 7;
                }
                else
                {
                    auto slack = Sqrx(steerComp.Slack);

                    auto vecToGoal = steerComp.GoalPos - currPos;
                    auto distToGoal = vecToGoal.Length();
                    bool goalInRange = distToGoal < steerComp.ArrivalRange;

                    bool stalled = false;

                    if (Vec2::Equals(steerComp.StepPos[0], steerComp.GoalPos) ||
                        HasAnyFlags(steerComp.Flags, ESteerFlags::FailedPathPlan))
                    {
                        if (distToGoal < slack)
                        {
                            stalled = true;
                        }
                    }
                    else if (Vec2::Equals(steerComp.StepPos[1], steerComp.GoalPos))
                    {
                        if (distToGoal < slack && distToStep0 < slack &&
                            (currPos - steerComp.StepPos[0]).Length() + (steerComp.StepPos[0] + steerComp.StepPos[1]).Length() < slack)
                        {
                            stalled = true;
                        }
                    }

                    bool arrived = false;
                    if (stalled)
                    {
                        steerComp.Mode = ESteerMode::Idle;
                        ClearFlagRef(steerComp.Flags, ESteerFlags::SeekingGoal);
                        ClearFlagRef(steerComp.Flags, ESteerFlags::ArrivedAtGoal);
                    }
                    else
                    {
                        arrived = goalInRange;
                    }

                    if (arrived)
                    {
                        if (distToStep0 < steerComp.MaxSpeed)
                        {
                            vel = distToStep0;
                            if (Vec2::Equals(currPos, steerComp.GoalPos))
                            {
                                steerComp.Mode = ESteerMode::Idle;
                                SetFlagRef(steerComp.Flags, ESteerFlags::ArrivedAtGoal);
                            }
                        }
                        else
                        {
                            vel /= 2;
                            if (vel.Length() <= steerComp.MaxSpeed)
                            {
                                vel = Vec2::Zero;
                                steerComp.Mode = ESteerMode::Idle;
                                SetFlagRef(steerComp.Flags, ESteerFlags::ArrivedAtGoal);
                            }
                        }
                    }
                    else
                    {
                        // Calculate the velocity vector the moves the entity towards the goal.
                        vel = CalculateVelocity(transformComp, steerComp);
                    }
                }

                steerComp.Velocity = vel;

                // Rotate to face goal direction
                if (!HasAnyFlags(steerComp.Flags, ESteerFlags::LockFacing))
                {
                    CalculateFacing(transformComp, steerComp, vecToStep0, transformComp.Transform.Rotation);
                }

                CalculateSlack(transformComp, steerComp);

                // Integrate velocity
                Integrate(transformComp, steerComp);
            }
        }

        Vec2 CalculateVelocity(const TransformComponent& transformComp, const SteeringComponent& steerComp) const
        {
            // Only step entities actively seeking a goal
            if (!HasAnyFlags(steerComp.Flags, ESteerFlags::SeekingGoal))
            {
                return Vec2::Zero;
            }

            const Vec2& currPos = transformComp.Transform.Position;
            const Vec2& stepPos = steerComp.StepPos[0];
            Vec2 stepDir = stepPos - currPos;

            // Wait to start moving until the entity has rotated to face the step pos
            if (0)
            {
                auto targetAngle = stepDir.AsRadians();
                auto delta = AngleDelta(targetAngle, transformComp.Transform.Rotation);
                if (Abs(delta) > Rad45)
                {
                    return Vec2::Zero;
                }
            }

            Distance dist = stepDir.Length() / DeltaTime;
            auto speed = Min(steerComp.MaxSpeed, dist);
            return stepDir.Normalized() * speed;
        }

        Vec2 CalculateVelocity2(const TransformComponent& transformComp, const SteeringComponent& steerComp) const
        {
            // Only step entities actively seeking a goal
            if (!HasAnyFlags(steerComp.Flags, ESteerFlags::SeekingGoal))
            {
                return Vec2::Zero;
            }

            const Vec2& currPos = transformComp.Transform.Position;
            const Vec2& stepPos = steerComp.StepPos[0];
            Vec2 stepDir = stepPos - currPos;

            // Wait to start moving until the entity has rotated to face the step pos
            if (0)
            {
                auto targetAngle = stepDir.AsRadians();
                auto delta = AngleDelta(targetAngle, transformComp.Transform.Rotation);
                if (Abs(delta) > Rad45)
                {
                    return Vec2::Zero;
                }
            }

            Vec2 v = currPos - steerComp.PreviousPos;
            int64 vv = Vec2::SqrxQ(v);

            Vec2 d = stepPos - currPos;
            int64 dd = Vec2::SqrxQ(d);

            int64 speed = steerComp.MaxSpeed.Value;
            int64 speed2 = speed*speed;

            int64 decel = 1;
            if (steerComp.DecelerationTime.Value > 0)
            {
                decel = (int64)steerComp.MaxSpeed.Value / steerComp.DecelerationTime.Value;
            }
            if (decel < 1)
            {
                decel = 1;
            }
            int64 decel2 = decel*decel;

            int64 accel2 = speed2;
            if (steerComp.AccelerationTime.Value > 0)
            {
                accel2 = speed2 / steerComp.AccelerationTime.Value;
            }
            if (accel2 < 2)
            {
                accel2 = 2;
            }

            int64 stopTime2 = vv / decel2;
            int64 stopDist2 = stopTime2 * vv;

            if (dd <= accel2)
            {
                v = d;
                vv = dd;
            }
            else if (dd <= stopDist2)
            {
                int64 decelNeed = (vv*vv) / dd;
                if (decelNeed > decel2)
                    decel2 = decelNeed;

                if (vv > decel2)
                {
                    v -= v.Normalized() * Distance(Q64(decel2));
                    vv = Vec2::SqrxQ(v);
                }
            }
            else
            {
                if (vv < speed2)
                {
                    vv = vv * 9/8 + accel2;
                }
                if (vv > speed2)
                {
                    vv = speed2;
                }
            }

            v = vv >= dd ? d : d.Normalized() * Distance(Q64(vv));
            return v;
        }

        bool CalculateFacing(
            const TransformComponent& transformComp,
            const SteeringComponent& steerComp,
            const Vec2& velocity,
            Angle& outAngle,
            Angle threshold = Angle::Epsilon) const
        {
            Distance len = velocity.Length();
            Distance thresh = Distance(Q32(128));
            if (len <= thresh)
            {
                return false;
            }

            Angle angle = velocity.AsRadians();
            Angle delta = AngleDelta(angle, transformComp.Transform.Rotation);
            Angle absDelta = Abs(delta);

            if (absDelta <= threshold)
            {
                return false;
            }

            if (steerComp.TurnRateMoving == 0)
            {
                return false;
            }

            Angle turnRate = Pi / steerComp.TurnRateMoving * DeltaTime;

            if (absDelta < turnRate)
            {
                outAngle = angle;
            }
            else if (delta > 0)
            {
                outAngle = Cordic::AngleShift(transformComp.Transform.Rotation + turnRate);
            }
            else
            {
                outAngle = Cordic::AngleShift(transformComp.Transform.Rotation - turnRate);
            }

            return true;
        }

        void CalculateSlack(const TransformComponent& transformComp, SteeringComponent& steerComp) const
        {
            Vec2 dest = steerComp.StepPos[0];
            Vec2 currToDest = dest - transformComp.Transform.Position;
            Vec2 bestToDest = dest - steerComp.BestPos;

            if (currToDest.Length() < bestToDest.Length())
            {
                steerComp.BestPos = transformComp.Transform.Position;
                steerComp.Slack /= 2;
                if (steerComp.Slack < steerComp.InnerRadius)
                {
                    steerComp.Slack = 0;
                }
                return;
            }

            std::vector<const SortedEntity*> entities;
            SteeringRangeQueryArgs queryArgs = { steerComp.CollisionMask };
            FeatureSteering::QueryEntitiesInRange(*World, steerComp.GoalPos, steerComp.Slack, entities);

            uint64 crowdedness = 0;
            for (const SortedEntity* entity : entities)
            {
                crowdedness += SqrxQ(entity->SteeringComponent->OuterRadius);
            }

            Value increaseRate = 1.0;
            if (steerComp.Slack > 0)
            {
                uint64 slackArea = SqrxQ(steerComp.Slack);
                if (crowdedness > slackArea)
                {
                    crowdedness = slackArea;
                }

                increaseRate = Lerp01<Value>(SlackIncreaseRate, SlackIncreaseRateFast, crowdedness / slackArea);
            }

            if (steerComp.Mode == ESteerMode::Move)
            {
                steerComp.Slack += steerComp.InnerRadius * increaseRate / SlackRateDivisor;
            }
            else
            {
                steerComp.Slack += steerComp.InnerRadius * increaseRate / SlackRateDivisorSlow;
            }

            if (steerComp.Slack > MaxSlack)
            {
                steerComp.Slack = MaxSlack; 
            }
        }

        void Integrate(TransformComponent& transformComp, SteeringComponent& steerComp)
        {
            Distance vel = steerComp.Velocity.Length();
            vel = Min(vel, steerComp.MaxSpeed);
            steerComp.Velocity = steerComp.Velocity.Normalized() * vel;

            steerComp.PreviousPos = transformComp.Transform.Position;
            transformComp.Transform.Position += steerComp.Velocity * DeltaTime;

            CollideWithWorld(transformComp, steerComp);

            bool moved = !Vec2::Equals(steerComp.PreviousPos, transformComp.Transform.Position);
            SetFlagRef(steerComp.Flags, ESteerFlags::Active, moved);
        }

        void CollideWithWorld(
            TransformComponent& transformComp,
            SteeringComponent& steerComp)
        {
            
        }
    };

    struct CollisionJob : IBufferJob<TransformComponent&, SteeringComponent&>
    {
        void Execute(const EntityComponentSpan<TransformComponent&, SteeringComponent&>& span) override
        {
            PHX_PROFILE_ZONE_SCOPED_N("CollisionJob");

            FeatureSteeringScratchBlock& scratchBlock = World->GetBlockRef<FeatureSteeringScratchBlock>();

            TMortonCodeRangeArray ranges;
            const SortedEntity* neighbors[64];
            uint32 neighborsCount = 0;

            for (auto && [
                entityId,
                index,
                transformCompA,
                steerCompA] : span)
            {
                if (HasNoneFlags(steerCompA.Flags, ESteerFlags::Active))
                {
                    continue;
                }

                // Query for overlapping morton ranges
                {
                    PHX_PROFILE_ZONE_SCOPED_N("OverlapQuery");

                    Distance range = Max(steerCompA.AvoidanceRadius, scratchBlock.MaxEntityRadius);
                    MortonCodeAABB aabb = ToMortonCodeAABB(transformCompA.Transform.Position, range);

                    ranges.clear();
                    MortonCodeQuery(aabb, ranges);

                    neighborsCount = 0;
                    ForEachInMortonCodeRanges<SortedEntity, &SortedEntity::ZCode>(
                        scratchBlock.SortedEntities,
                        ranges,
                        [&](const SortedEntity& other)
                        {
                            if (other.EntityId == entityId)
                            {
                                return false;
                            }

                            if ((steerCompA.CollisionMask & other.SteeringComponent->CollisionMask) == 0)
                            {
                                return false;
                            }

                            neighbors[neighborsCount++] = &other;
                            return neighborsCount == _countof(neighbors);
                        });
                }

                for (uint32 i = 0; i < neighborsCount; ++i)
                {
                    const SortedEntity* entityB = neighbors[i];
                    TransformComponent& transformCompB = *entityB->TransformComponent;
                    SteeringComponent& steerCompB = *entityB->SteeringComponent;

                    auto radius = steerCompA.OuterRadius + steerCompB.OuterRadius;
                    auto rr = SqrxQ(radius);

                    auto dist = transformCompB.Transform.Position - transformCompA.Transform.Position;
                    auto dd = Vec2::SqrxQ(dist);

                    if (dd >= rr)
                    {
                        continue;
                    }

                    if (dd == 0)
                    {
                        dist = Vec2(radius, 0);
                        dd = rr;
                    }

                    Value separationRatioA = 1.0;
                    Value separationRatioB = 1.0;

                    if (HasAnyFlags(steerCompA.Flags, ESteerFlags::Holding))
                    {
                        separationRatioA = 0.0;
                    }
                    if (HasAnyFlags(steerCompB.Flags, ESteerFlags::Holding))
                    {
                        separationRatioB = 0.0;
                    }

                    Value totalRatio = separationRatioA + separationRatioB;
                    if (totalRatio != 0)
                    {
                        auto normal = dist.Normalized() * radius;
                        auto separation = normal - dist;

                        if (separationRatioA != 0)
                        {
                            transformCompA.Transform.Position -= separation * separationRatioA / totalRatio;
                            SetFlagRef(steerCompA.Flags, ESteerFlags::Active);
                        }

                        if (separationRatioB != 0)
                        {
                            transformCompB.Transform.Position += separation * separationRatioB / totalRatio;
                            SetFlagRef(steerCompB.Flags, ESteerFlags::Active);
                        }
                    }
                }
            }
        }
    };
}

void SteeringSystem::OnPreWorldUpdate(WorldRef world, const SystemUpdateArgs& args)
{
    FeatureSteeringScratchBlock& scratchBlock = world.GetBlockRef<FeatureSteeringScratchBlock>();

    scratchBlock.SortedEntities.Reset();
    scratchBlock.SortedEntityCount = 0;

    SteeringDetail::PopulateSortedEntitiesJob job;
    FeatureECS::ScheduleParallel(world, job);

    WorldTaskQueue::Schedule(world, &SteeringDetail::SortEntitiesByZCodeTask);
}

void SteeringSystem::OnWorldUpdate(WorldRef world, const SystemUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    SteeringDetail::PathfindingJob pathfindingJob;
    pathfindingJob.ArrivalThreshold = ArrivalThreshold;
    FeatureECS::ScheduleParallel(world, pathfindingJob);

    SteeringDetail::SteeringJob steeringJob;
    steeringJob.DeltaTime = args.DeltaTime;
    steeringJob.MoveTowardsGoal = MoveTowardsGoal;
    steeringJob.ArrivalThreshold = ArrivalThreshold;
    steeringJob.DensityScalar = DensityScalar;
    steeringJob.DensityRadiusScalar = DensityRadiusScalar;
    steeringJob.AvoidanceScalar = AvoidanceScalar;
    steeringJob.AvoidanceRadiusScalar = AvoidanceRadiusScalar;
    steeringJob.SlackIncreaseRate = SlackIncreaseRate;
    steeringJob.SlackIncreaseRateFast = SlackIncreaseRateFast;
    steeringJob.SlackRateDivisor = SlackRateDivisor;
    steeringJob.SlackRateDivisorSlow = SlackRateDivisorSlow;
    steeringJob.MaxSlack = MaxSlack;
    FeatureECS::ScheduleParallel(world, steeringJob);

    for (uint32 i = 0; i < 2; ++i)
    {
        SteeringDetail::CollisionJob collisionJob;
        FeatureECS::ScheduleParallel(world, collisionJob);
    }
}

void SteeringSystem::OnDebugRender(
    WorldConstRef world,
    const IDebugState& state,
    IDebugRenderer& renderer)
{
    ISystem::OnDebugRender(world, state, renderer);

    // EntityQueryBuilder builder;
    // builder.RequireAllComponents<TransformComponent, BodyComponent, SteeringComponent>();
    // auto query = builder.GetQuery();
    //
    // FeatureECS::ForEachEntity(world, query, std::function([&](const EntityComponentSpan<const TransformComponent&, const BodyComponent&, const SteeringComponent&>& span)
    // {
    //     for (auto && [entity, index, transformComp, bodyComp, steeringComp] : span)
    //     {
    //         Vec2 start = transformComp.Transform.Position;
    //
    //         Vec2 dir = steeringComp.SteeringVector * bodyComp.InvMass * OneDivBy(Distance(60.0));
    //         renderer.DrawRay(start, dir, Color::Blue);
    //
    //         dir = steeringComp.GoalVector * bodyComp.InvMass * OneDivBy(Distance(60.0));
    //         renderer.DrawRay(start, dir, Color::Green);
    //
    //         dir = steeringComp.AvoidVector * bodyComp.InvMass * OneDivBy(Distance(60.0));
    //         renderer.DrawRay(start, dir, Color::Red);
    //
    //         dir = steeringComp.DensityVector * bodyComp.InvMass * OneDivBy(Distance(60.0));
    //         renderer.DrawRay(start, dir, Color::Yellow);
    //     }
    // }));
}

// ── Type registration ──────────────────────────────────────────────────────────

PHX_DEFINE_TYPE(SteeringSystem)
{
    registration
        .Field("MoveTowardsGoal",       &SteeringSystem::MoveTowardsGoal)
        .Field("DensityScalar",         &SteeringSystem::DensityScalar)
        .Field("DensityRadiusScalar",   &SteeringSystem::DensityRadiusScalar)
        .Field("AvoidanceScalar",       &SteeringSystem::AvoidanceScalar)
        .Field("AvoidanceRadiusScalar", &SteeringSystem::AvoidanceRadiusScalar)
        .Field("ArrivalThreshold",      &SteeringSystem::ArrivalThreshold)
        .Field("SlackIncreaseRate",     &SteeringSystem::SlackIncreaseRate)
        .Field("SlackIncreaseRateFast", &SteeringSystem::SlackIncreaseRateFast)
        .Field("SlackRateDivisor",      &SteeringSystem::SlackRateDivisor)
        .Field("SlackRateDivisorSlow",  &SteeringSystem::SlackRateDivisorSlow)
        .Field("MaxSlack",              &SteeringSystem::MaxSlack);
}
