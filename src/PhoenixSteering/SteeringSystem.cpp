
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

namespace Phoenix::Steering::SteeringDetail
{
    struct PopulateSortedEntitiesJob : IJob<TransformComponent&, SteeringComponent&>
    {
        FeatureSteeringScratchBlock* ScratchBlock = nullptr;

        void Execute(WorldConstRef /*world*/, EntityId entityId, CommandBuffer& /*cb*/,
                     TransformComponent& transformComp, SteeringComponent& steeringComp) override
        {
            uint32 sortedEntityIndex = ScratchBlock->SortedEntityCount.fetch_add(1);
            ScratchBlock->SortedEntities[sortedEntityIndex] =
                SortedEntity{entityId, &transformComp, &steeringComp, transformComp.ZCode};
        }
    };

    struct SortEntitiesByZCodeTask : ITask
    {
        FeatureSteeringScratchBlock* ScratchBlock = nullptr;

        FName GetName() const override { return "SortSteeringEntitiesByZCode"_n; }

        void Run(WorldConstRef /*world*/, CommandBuffer& /*cb*/) override
        {
            ScratchBlock->SortedEntities.SetSize(ScratchBlock->SortedEntityCount);

            std::ranges::stable_sort(
                ScratchBlock->SortedEntities,
                [](const SortedEntity& a, const SortedEntity& b)
                {
                    return a.ZCode < b.ZCode;
                });

            ScratchBlock->MaxEntityRadius = 0;
            for (const SortedEntity& entity : ScratchBlock->SortedEntities)
            {
                if (entity.SteeringComponent->OuterRadius > ScratchBlock->MaxEntityRadius)
                    ScratchBlock->MaxEntityRadius = entity.SteeringComponent->OuterRadius;
            }
        }
    };

    struct PathfindingJob : IJob<TransformComponent&, SteeringComponent&>
    {
        Distance ArrivalThreshold;

        void Execute(WorldConstRef world, EntityId /*entityId*/, CommandBuffer& /*cb*/,
                     TransformComponent& transformComp, SteeringComponent& steerComp) override
        {
            PHX_PROFILE_ZONE_SCOPED_N("PathfindingJob");
            DetermineStepPositions(world, transformComp, steerComp);
        }

        void DetermineStepPositions(WorldConstRef world, const TransformComponent& transformComp, SteeringComponent& steerComp) const
        {
            PHX_PROFILE_ZONE_SCOPED;

            if (!HasAnyFlags(steerComp.Flags, ESteerFlags::SeekingGoal))
                return;

            bool clearSeekingGoal = true;
            if (steerComp.GoalEntity != EntityId::Invalid)
            {
                if (const Transform2D* targetTransform = FeatureECS::GetWorldTransformPtr(world, steerComp.GoalEntity))
                {
                    steerComp.GoalPos = targetTransform->Position;
                    clearSeekingGoal = false;
                }
                else
                {
                    steerComp.GoalEntity = EntityId::Invalid;
                    ClearFlagRef(steerComp.Flags, ESteerFlags::SeekingGoal);
                    return;
                }
            }

            Vec2 targetPos = steerComp.GoalPos;
            const Transform2D& transform = transformComp.Transform;
            const Vec2& currPos = transform.Position;

            Vec2 targetOffset = targetPos - currPos;
            Distance distance = targetOffset.Length();
            Distance arrivalRange = Max(ArrivalThreshold, steerComp.ArrivalRange);

            if (distance < arrivalRange || distance < steerComp.Slack)
            {
                SetFlagRef(steerComp.Flags, ESteerFlags::ArrivedAtGoal);
                if (clearSeekingGoal)
                    ClearFlagRef(steerComp.Flags, ESteerFlags::SeekingGoal);
            }

            steerComp.StepPos[0] = targetPos;
        }
    };

    struct SteeringJob : IJob<TransformComponent&, SteeringComponent&>
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

        void Execute(WorldConstRef world, EntityId /*entityId*/, CommandBuffer& /*cb*/,
                     TransformComponent& transformComp, SteeringComponent& steerComp) override
        {
            PHX_PROFILE_ZONE_SCOPED_N("SteeringJob");

            if (!HasAnyFlags(steerComp.Flags, ESteerFlags::SeekingGoal))
                return;

            const Vec2& currPos = transformComp.Transform.Position;
            Vec2 vel = currPos - steerComp.PreviousPos;

            auto vecToStep0 = steerComp.StepPos[0] - currPos;
            auto distToStep0 = vecToStep0.Length();

            if (steerComp.Mode == ESteerMode::Idle)
            {
                while (vel.Length() > steerComp.MaxSpeed)
                    vel /= 2;
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
                        stalled = true;
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
                    vel = CalculateVelocity(transformComp, steerComp);
                }
            }

            steerComp.Velocity = vel;

            if (!HasAnyFlags(steerComp.Flags, ESteerFlags::LockFacing))
                CalculateFacing(transformComp, steerComp, vecToStep0, transformComp.Transform.Rotation);

            CalculateSlack(world, transformComp, steerComp);
            Integrate(transformComp, steerComp);
        }

        Vec2 CalculateVelocity(const TransformComponent& transformComp, const SteeringComponent& steerComp) const
        {
            if (!HasAnyFlags(steerComp.Flags, ESteerFlags::SeekingGoal))
                return Vec2::Zero;

            const Vec2& currPos = transformComp.Transform.Position;
            const Vec2& stepPos = steerComp.StepPos[0];
            Vec2 stepDir = stepPos - currPos;

            Distance dist = stepDir.Length() / DeltaTime;
            auto speed = Min(steerComp.MaxSpeed, dist);
            return stepDir.Normalized() * speed;
        }

        Vec2 CalculateVelocity2(const TransformComponent& transformComp, const SteeringComponent& steerComp) const
        {
            if (!HasAnyFlags(steerComp.Flags, ESteerFlags::SeekingGoal))
                return Vec2::Zero;

            const Vec2& currPos = transformComp.Transform.Position;
            const Vec2& stepPos = steerComp.StepPos[0];
            Vec2 stepDir = stepPos - currPos;

            Vec2 v = currPos - steerComp.PreviousPos;
            int64 vv = Vec2::SqrxQ(v);

            Vec2 d = stepPos - currPos;
            int64 dd = Vec2::SqrxQ(d);

            int64 speed = steerComp.MaxSpeed.Value;
            int64 speed2 = speed * speed;

            int64 decel = 1;
            if (steerComp.DecelerationTime.Value > 0)
                decel = (int64)steerComp.MaxSpeed.Value / steerComp.DecelerationTime.Value;
            if (decel < 1)
                decel = 1;
            int64 decel2 = decel * decel;

            int64 accel2 = speed2;
            if (steerComp.AccelerationTime.Value > 0)
                accel2 = speed2 / steerComp.AccelerationTime.Value;
            if (accel2 < 2)
                accel2 = 2;

            int64 stopTime2 = vv / decel2;
            int64 stopDist2 = stopTime2 * vv;

            if (dd <= accel2)
            {
                v = d;
                vv = dd;
            }
            else if (dd <= stopDist2)
            {
                int64 decelNeed = (vv * vv) / dd;
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
                    vv = vv * 9 / 8 + accel2;
                if (vv > speed2)
                    vv = speed2;
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
                return false;

            Angle angle = velocity.AsRadians();
            Angle delta = AngleDelta(angle, transformComp.Transform.Rotation);
            Angle absDelta = Abs(delta);

            if (absDelta <= threshold)
                return false;

            if (steerComp.TurnRateMoving == 0)
                return false;

            Angle turnRate = Pi / steerComp.TurnRateMoving * DeltaTime;

            if (absDelta < turnRate)
                outAngle = angle;
            else if (delta > 0)
                outAngle = Cordic::AngleShift(transformComp.Transform.Rotation + turnRate);
            else
                outAngle = Cordic::AngleShift(transformComp.Transform.Rotation - turnRate);

            return true;
        }

        void CalculateSlack(WorldConstRef world, const TransformComponent& transformComp, SteeringComponent& steerComp) const
        {
            Vec2 dest = steerComp.StepPos[0];
            Vec2 currToDest = dest - transformComp.Transform.Position;
            Vec2 bestToDest = dest - steerComp.BestPos;

            if (currToDest.Length() < bestToDest.Length())
            {
                steerComp.BestPos = transformComp.Transform.Position;
                steerComp.Slack /= 2;
                if (steerComp.Slack < steerComp.InnerRadius)
                    steerComp.Slack = 0;
                return;
            }

            std::vector<const SortedEntity*> entities;
            SteeringRangeQueryArgs queryArgs = { steerComp.CollisionMask };
            FeatureSteering::QueryEntitiesInRange(world, steerComp.GoalPos, steerComp.Slack, entities);

            uint64 crowdedness = 0;
            for (const SortedEntity* entity : entities)
                crowdedness += SqrxQ(entity->SteeringComponent->OuterRadius);

            Value increaseRate = 1.0;
            if (steerComp.Slack > 0)
            {
                uint64 slackArea = SqrxQ(steerComp.Slack);
                if (crowdedness > slackArea)
                    crowdedness = slackArea;
                increaseRate = Lerp01<Value>(SlackIncreaseRate, SlackIncreaseRateFast, crowdedness / slackArea);
            }

            if (steerComp.Mode == ESteerMode::Move)
                steerComp.Slack += steerComp.InnerRadius * increaseRate / SlackRateDivisor;
            else
                steerComp.Slack += steerComp.InnerRadius * increaseRate / SlackRateDivisorSlow;

            if (steerComp.Slack > MaxSlack)
                steerComp.Slack = MaxSlack;
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

        void CollideWithWorld(TransformComponent& /*transformComp*/, SteeringComponent& /*steerComp*/)
        {
        }
    };

    struct CollisionJob : IJob<TransformComponent&, SteeringComponent&>
    {
        FeatureSteeringScratchBlock* ScratchBlock = nullptr;

        void Execute(WorldConstRef /*world*/, EntityId entityId, CommandBuffer& /*cb*/,
                     TransformComponent& transformCompA, SteeringComponent& steerCompA) override
        {
            PHX_PROFILE_ZONE_SCOPED_N("CollisionJob");

            if (HasNoneFlags(steerCompA.Flags, ESteerFlags::Active))
                return;

            TMortonCodeRangeArray ranges;
            const SortedEntity* neighbors[64];
            uint32 neighborsCount = 0;

            // Query for overlapping morton ranges
            {
                PHX_PROFILE_ZONE_SCOPED_N("OverlapQuery");

                Distance range = Max(steerCompA.AvoidanceRadius, ScratchBlock->MaxEntityRadius);
                MortonCodeAABB aabb = ToMortonCodeAABB(transformCompA.Transform.Position, range);

                ranges.clear();
                MortonCodeQuery(aabb, ranges);

                neighborsCount = 0;
                ForEachInMortonCodeRanges<SortedEntity, &SortedEntity::ZCode>(
                    ScratchBlock->SortedEntities,
                    ranges,
                    [&](const SortedEntity& other)
                    {
                        if (other.EntityId == entityId)
                            return false;
                        if ((steerCompA.CollisionMask & other.SteeringComponent->CollisionMask) == 0)
                            return false;
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
                    continue;

                if (dd == 0)
                {
                    dist = Vec2(radius, 0);
                    dd = rr;
                }

                Value separationRatioA = 1.0;
                Value separationRatioB = 1.0;

                if (HasAnyFlags(steerCompA.Flags, ESteerFlags::Holding))
                    separationRatioA = 0.0;
                if (HasAnyFlags(steerCompB.Flags, ESteerFlags::Holding))
                    separationRatioB = 0.0;

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
    };
}

void SteeringSystem::OnWorldInitialize(WorldRef world)
{
    FeatureSteeringScratchBlock& scratchBlock = world.GetBlockRef<FeatureSteeringScratchBlock>();
    const FeatureECSDynamicBlock& ecsBlock = world.GetBlockRef<FeatureECSDynamicBlock>();

    // --- Pre-update scheduler: populate + sort ---
    {
        auto populate = std::make_unique<SteeringDetail::PopulateSortedEntitiesJob>();
        populate->ScratchBlock = &scratchBlock;

        auto sort = std::make_unique<SteeringDetail::SortEntitiesByZCodeTask>();
        sort->ScratchBlock = &scratchBlock;

        JobHandle hPopulate = PreUpdateScheduler.RegisterJob(std::move(populate));
        JobHandle hSort     = PreUpdateScheduler.RegisterJob(std::move(sort));
        PreUpdateScheduler.AddDependency(hSort, hPopulate);
        PreUpdateScheduler.Build(ecsBlock.ArchetypeManager);
    }

    // --- Update scheduler: pathfinding → steering → collision × 2 ---
    {
        auto pathfinding = std::make_unique<SteeringDetail::PathfindingJob>();
        PathfindingJobPtr = pathfinding.get();

        auto steering = std::make_unique<SteeringDetail::SteeringJob>();
        SteeringJobPtr = steering.get();

        auto collision0 = std::make_unique<SteeringDetail::CollisionJob>();
        collision0->ScratchBlock = &scratchBlock;
        CollisionJobPtr[0] = collision0.get();

        auto collision1 = std::make_unique<SteeringDetail::CollisionJob>();
        collision1->ScratchBlock = &scratchBlock;
        CollisionJobPtr[1] = collision1.get();

        JobHandle hPathfinding = UpdateScheduler.RegisterJob(std::move(pathfinding));
        JobHandle hSteering    = UpdateScheduler.RegisterJob(std::move(steering));
        JobHandle hCollision0  = UpdateScheduler.RegisterJob(std::move(collision0));
        JobHandle hCollision1  = UpdateScheduler.RegisterJob(std::move(collision1));

        // Explicit ordering: pathfinding → steering → collision0 → collision1
        UpdateScheduler.AddDependency(hSteering,   hPathfinding);
        UpdateScheduler.AddDependency(hCollision0, hSteering);
        UpdateScheduler.AddDependency(hCollision1, hCollision0);
        UpdateScheduler.Build(ecsBlock.ArchetypeManager);
    }
}

void SteeringSystem::OnPreWorldUpdate(WorldRef world, const SystemUpdateArgs& /*args*/)
{
    FeatureSteeringScratchBlock& scratchBlock = world.GetBlockRef<FeatureSteeringScratchBlock>();
    scratchBlock.SortedEntities.Reset();
    scratchBlock.SortedEntityCount = 0;

    FeatureECS::ExecuteScheduler(world, PreUpdateScheduler);
}

void SteeringSystem::OnWorldUpdate(WorldRef world, const SystemUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    PathfindingJobPtr->ArrivalThreshold = ArrivalThreshold;

    SteeringJobPtr->DeltaTime            = args.DeltaTime;
    SteeringJobPtr->MoveTowardsGoal      = MoveTowardsGoal;
    SteeringJobPtr->ArrivalThreshold     = ArrivalThreshold;
    SteeringJobPtr->DensityScalar        = DensityScalar;
    SteeringJobPtr->DensityRadiusScalar  = DensityRadiusScalar;
    SteeringJobPtr->AvoidanceScalar      = AvoidanceScalar;
    SteeringJobPtr->AvoidanceRadiusScalar = AvoidanceRadiusScalar;
    SteeringJobPtr->SlackIncreaseRate    = SlackIncreaseRate;
    SteeringJobPtr->SlackIncreaseRateFast = SlackIncreaseRateFast;
    SteeringJobPtr->SlackRateDivisor     = SlackRateDivisor;
    SteeringJobPtr->SlackRateDivisorSlow = SlackRateDivisorSlow;
    SteeringJobPtr->MaxSlack             = MaxSlack;

    FeatureECS::ExecuteScheduler(world, UpdateScheduler);
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
