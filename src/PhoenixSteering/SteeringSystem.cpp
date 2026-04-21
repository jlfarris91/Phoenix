
#include <algorithm>

#include "PhoenixSteering/SteeringSystem.h"

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
    struct ResetScratchBlockTask : ITask
    {
        const char* GetName() const override { return "Steering.ResetScratchBlockJob"; }

        void Run(WorldConstRef world, CommandBuffer& /*cb*/) override
        {
            auto* ecsScratch = world.GetBlock<FeatureECSScratchBlock>();
            auto* steeringScratch = const_cast<FeatureSteeringScratchBlock*>(world.GetBlock<FeatureSteeringScratchBlock>());

            // Invalidate scatter slots up to the high-water mark from the previous frame.
            // PrevSortedEntityCount is initialized to full capacity so frame 0 is safe.
            // Scatter only writes to positions 0..ecsCount-1, so this covers exactly the live range.
            std::memset(steeringScratch->SortedEntities.GetData(), 0xFF,
                        ecsScratch->PrevSortedEntityCount * sizeof(SortedEntity));

            steeringScratch->SortedEntityCount = 0;
        }
    };

    // Scatter each steering entity directly into its ECS-sorted position.
    // Requires SortedEntityIndex to be populated by the ECS sort task.
    // Uses read-only component access so it can run in parallel with other read-only jobs.
    struct ScatterSortedEntitiesJob : IJob<const TransformComponent&, const SteeringComponent&>
    {
        const FeatureECSScratchBlock*  EcsScratch      = nullptr;
        FeatureSteeringScratchBlock*   SteeringScratch = nullptr;

        const char* GetName() const override { return "Steering.ScatterSortedEntitiesJob"; }

        void BeginBatch(WorldConstRef world, const JobBatch&, CommandBuffer&) override
        {
            EcsScratch      = world.GetBlock<FeatureECSScratchBlock>();
            SteeringScratch = const_cast<FeatureSteeringScratchBlock*>(world.GetBlock<FeatureSteeringScratchBlock>());
        }

        void Execute(WorldConstRef /*world*/,
                     EntityId entityId,
                     CommandBuffer& /*cb*/,
                     const TransformComponent& transformComp,
                     const SteeringComponent& steeringComp) override
        {
            const uint32 entityIdx = (entityId % EcsScratch->SortedEntityIndex.GetCapacity()) - 1;
            const uint32 sortedIdx = EcsScratch->SortedEntityIndex[entityIdx];
            if (sortedIdx != Index<uint32>::None)
            {
                SteeringScratch->SortedEntities[sortedIdx] = {
                    .EntityId = entityId,
                    .TransformComponent = const_cast<TransformComponent*>(&transformComp),
                    .SteeringComponent = const_cast<SteeringComponent*>(&steeringComp),
                    .ZCode = transformComp.ZCode
                };
            }
        }
    };

    // Compact the scatter buffer into a dense sorted array and compute MaxEntityRadius.
    struct CompactSortedEntitiesTask : ITask
    {
        const char* GetName() const override { return "Steering.CompactSortedEntitiesTask"; }

        void Run(WorldConstRef world, CommandBuffer& /*cb*/) override
        {
            const auto* ecsScratch = world.GetBlock<FeatureECSScratchBlock>();
            auto* scratch = const_cast<FeatureSteeringScratchBlock*>(world.GetBlock<FeatureSteeringScratchBlock>());
            const uint32 ecsCount = ecsScratch->SortedEntities.GetNum();
            SortedEntity* data = scratch->SortedEntities.GetData();

            uint32 count = 0;
            for (uint32 i = 0; i < ecsCount; ++i)
            {
                if (data[i].EntityId != EntityId::Invalid)
                {
                    data[count++] = data[i];
                }
            }

            scratch->SortedEntities.SetSize(count);
            scratch->SortedEntityCount = count;

            scratch->MaxEntityRadius = 0;
            for (uint32 i = 0; i < count; ++i)
            {
                if (data[i].SteeringComponent->OuterRadius > scratch->MaxEntityRadius)
                {
                    scratch->MaxEntityRadius = data[i].SteeringComponent->OuterRadius;
                }
            }
        }
    };

    struct PathfindingJob : IJob<TransformComponent&, SteeringComponent&>
    {
        Distance ArrivalThreshold;

        const char* GetName() const override { return "Steering.PathfindingJob"; }

        void Execute(WorldConstRef world, EntityId /*entityId*/, CommandBuffer& /*cb*/,
                     TransformComponent& transformComp, SteeringComponent& steerComp) override
        {
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

        const char* GetName() const override { return "Steering.SteeringJob"; }

        void Execute(WorldConstRef world, EntityId /*entityId*/, CommandBuffer& /*cb*/,
                     TransformComponent& transformComp, SteeringComponent& steerComp) override
        {
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
            {
                return Vec2::Zero;
            }

            const Vec2& currPos = transformComp.Transform.Position;
            const Vec2& stepPos = steerComp.StepPos[0];
            // Vec2 stepDir = stepPos - currPos;

            Vec2 v = currPos - steerComp.PreviousPos;
            int64 vv = Vec2::SqrxQ(v);

            Vec2 d = stepPos - currPos;
            int64 dd = Vec2::SqrxQ(d);

            int64 speed = steerComp.MaxSpeed.Value;
            int64 speed2 = speed * speed;

            int64 decel = 1;
            if (steerComp.DecelerationTime.Value > 0)
            {
                decel = (int64)steerComp.MaxSpeed.Value / steerComp.DecelerationTime.Value;
            }
            decel = std::max<int64>(decel, 1);
            int64 decel2 = decel * decel;

            int64 accel2 = speed2;
            if (steerComp.AccelerationTime.Value > 0)
            {
                accel2 = speed2 / steerComp.AccelerationTime.Value;
            }
            accel2 = std::max<int64>(accel2, 2);

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
                decel2 = std::max(decelNeed, decel2);

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
                    vv = vv * 9 / 8 + accel2;
                }
                vv = std::min(vv, speed2);
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
                {
                    steerComp.Slack = 0;
                }
                return;
            }

            std::vector<const SortedEntity*> entities;
            SteeringRangeQueryArgs queryArgs = { .CollisionMask = steerComp.CollisionMask };
            FeatureSteering::QueryEntitiesInRange(world, steerComp.GoalPos, steerComp.Slack, entities);

            uint64 crowdedness = 0;
            for (const SortedEntity* entity : entities)
            {
                crowdedness += SqrxQ(entity->SteeringComponent->OuterRadius);
            }

            Value increaseRate = 1.0;
            if (steerComp.Slack > 0)
            {
                uint64 slackArea = SqrxQ(steerComp.Slack);
                crowdedness = std::min(crowdedness, slackArea);
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

        void CollideWithWorld(TransformComponent& /*transformComp*/, SteeringComponent& /*steerComp*/)
        {
        }
    };

    // ITask (not IJob) so it always runs as a single batch on one thread.
    // CollisionJob previously used IJob<TransformComponent&, SteeringComponent&>, but that
    // creates multiple per-archetype batches that run in parallel. Each batch writes to
    // neighbor entities' TransformComponent/SteeringComponent via SortedEntities pointers,
    // racing with other batches that process those same neighbors. Converting to ITask
    // eliminates the race without changing any separation logic.
    struct CollisionTask : ITask
    {
        const char* GetName() const override { return "Steering.CollisionTask"; }

        void Run(WorldConstRef world, CommandBuffer& /*cb*/) override
        {
            auto* scratch = const_cast<FeatureSteeringScratchBlock*>(world.GetBlock<FeatureSteeringScratchBlock>());

            TMortonCodeRangeArray ranges;
            const SortedEntity* neighbors[64];

            const uint32 count = scratch->SortedEntities.GetNum();
            for (uint32 idx = 0; idx < count; ++idx)
            {
                const SortedEntity& entityA = scratch->SortedEntities[idx];
                TransformComponent& transformCompA = *entityA.TransformComponent;
                SteeringComponent& steerCompA = *entityA.SteeringComponent;

                if (HasNoneFlags(steerCompA.Flags, ESteerFlags::Active))
                    continue;

                uint32 neighborsCount = 0;

                // Query for overlapping morton ranges
                {
                    PHX_PROFILE_ZONE_SCOPED_N("OverlapQuery");

                    Distance range = Max(steerCompA.AvoidanceRadius, scratch->MaxEntityRadius);
                    MortonCodeAABB aabb = ToMortonCodeAABB(transformCompA.Transform.Position, range);

                    ranges.clear();
                    MortonCodeQuery(aabb, ranges);

                    ForEachInMortonCodeRanges<SortedEntity, &SortedEntity::ZCode>(
                        scratch->SortedEntities,
                        ranges,
                        [&](const SortedEntity& other)
                        {
                            if (other.EntityId == entityA.EntityId)
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
                    {
                        continue;
                    }

                    if (dd == 0)
                    {
                        dist = Vec2(radius, 0);
                        // dd = rr;
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

SteeringSystem::SteeringSystem() = default;
SteeringSystem::~SteeringSystem() = default;

void SteeringSystem::OnWorldInitialize(WorldRef world)
{
    // --- Pre-update scheduler: reset → scatter (using ECS sorted index) → compact ---
    {
        auto reset   = std::make_unique<SteeringDetail::ResetScratchBlockTask>();
        auto scatter = std::make_unique<SteeringDetail::ScatterSortedEntitiesJob>();
        auto compact = std::make_unique<SteeringDetail::CompactSortedEntitiesTask>();

        JobHandle hReset   = FeatureECS::RegisterJob(world, std::move(reset),   EJobPhase::PreUpdate);
        JobHandle hScatter = FeatureECS::RegisterJob(world, std::move(scatter), EJobPhase::PreUpdate);
        JobHandle hCompact = FeatureECS::RegisterJob(world, std::move(compact), EJobPhase::PreUpdate);

        FeatureECS::AddJobDependency(world, EJobPhase::PreUpdate, hScatter, hReset);
        FeatureECS::AddJobDependency(world, EJobPhase::PreUpdate, hCompact, hScatter);

        // Scatter must run after ECS sort so SortedEntityIndex is populated
        JobHandle hECSSort = FeatureECS::GetPreUpdateSortJobHandle(world);
        FeatureECS::AddJobDependency(world, EJobPhase::PreUpdate, hScatter, hECSSort);
    }

    // --- Update scheduler: pathfinding → steering → collision × 2 ---
    {
        PathfindingJob  = std::make_unique<SteeringDetail::PathfindingJob>();
        SteeringJob     = std::make_unique<SteeringDetail::SteeringJob>();
        CollisionTask0  = std::make_unique<SteeringDetail::CollisionTask>();
        CollisionTask1  = std::make_unique<SteeringDetail::CollisionTask>();

        JobHandle hPathfinding = FeatureECS::RegisterJob(world, PathfindingJob.get(),  EJobPhase::Update);
        JobHandle hSteering    = FeatureECS::RegisterJob(world, SteeringJob.get(),     EJobPhase::Update);
        JobHandle hCollision0  = FeatureECS::RegisterJob(world, CollisionTask0.get(),  EJobPhase::Update);
        JobHandle hCollision1  = FeatureECS::RegisterJob(world, CollisionTask1.get(),  EJobPhase::Update);

        // Explicit ordering: pathfinding → steering → collision0 → collision1
        FeatureECS::AddJobDependency(world, EJobPhase::Update, hSteering, hPathfinding);
        FeatureECS::AddJobDependency(world, EJobPhase::Update, hCollision0, hSteering);
        FeatureECS::AddJobDependency(world, EJobPhase::Update, hCollision1, hCollision0);
    }
}

void SteeringSystem::OnPreWorldUpdate(WorldRef world, const SystemUpdateArgs& /*args*/)
{
}

void SteeringSystem::OnWorldUpdate(WorldRef world, const SystemUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    PathfindingJob->ArrivalThreshold = ArrivalThreshold;

    SteeringJob->DeltaTime            = args.DeltaTime;
    SteeringJob->MoveTowardsGoal      = MoveTowardsGoal;
    SteeringJob->ArrivalThreshold     = ArrivalThreshold;
    SteeringJob->DensityScalar        = DensityScalar;
    SteeringJob->DensityRadiusScalar  = DensityRadiusScalar;
    SteeringJob->AvoidanceScalar      = AvoidanceScalar;
    SteeringJob->AvoidanceRadiusScalar = AvoidanceRadiusScalar;
    SteeringJob->SlackIncreaseRate    = SlackIncreaseRate;
    SteeringJob->SlackIncreaseRateFast = SlackIncreaseRateFast;
    SteeringJob->SlackRateDivisor     = SlackRateDivisor;
    SteeringJob->SlackRateDivisorSlow = SlackRateDivisorSlow;
    SteeringJob->MaxSlack             = MaxSlack;
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
