
#include <algorithm>

#include "PhoenixSteering/SteeringSystem.h"

#include "PhoenixSim/Containers/FixedLeaderboard.h"
#include "PhoenixSim/Debug/Debug.h"
#include "PhoenixSim/Flags.h"
#include "PhoenixSim/MortonCode.h"
#include "PhoenixSim/WorldTaskQueue.h"
#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/ECS/SystemJob.h"
#include "PhoenixSim/ECS/TransformComponent.h"
#include "PhoenixSim/Navigation/FeatureNavigation.h"
#include "PhoenixPhysics/FeaturePhysics.h"
#include "PhoenixSim/Debug/DebugCommands.h"
#include "PhoenixSteering/FeatureSteering.h"
#include "PhoenixSteering/SteeringComponent.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::Physics;
using namespace Phoenix::Steering;

namespace Phoenix::Steering::SteeringDetail
{
    constexpr uint32 MaxEntitiesPerQuery = 8;

    struct ResetSortedEntitiesTask : ITask
    {
        const char* GetName() const override { return "Steering.ResetSortedEntities"; }

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

        const char* GetName() const override { return "Steering.ScatterSortedEntities"; }

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
        const char* GetName() const override { return "Steering.CompactSortedEntities"; }

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

        const char* GetName() const override { return "Steering.Pathfinding"; }

        void Execute(WorldConstRef world, EntityId /*entityId*/, CommandBuffer& /*cb*/,
                     TransformComponent& transformComp, SteeringComponent& steerComp) override
        {
            DetermineStepPositions(world, transformComp, steerComp);
        }

        void DetermineStepPositions(WorldConstRef world, const TransformComponent& transformComp, SteeringComponent& steerComp) const
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
                if (const Transform2D* targetTransform = FeatureECS::GetWorldTransformPtr(world, steerComp.GoalEntity))
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

            if (distance < arrivalRange || distance < steerComp.Slack)
            {
                SetFlagRef(steerComp.Flags, ESteerFlags::ArrivedAtGoal);
                if (clearSeekingGoal)
                    ClearFlagRef(steerComp.Flags, ESteerFlags::SeekingGoal);
            }

            steerComp.StepPos[0] = targetPos;
        }
    };

    enum class EBumpRatio : uint8
    {
        _100_0,
        _75_25,
        _50_50,
        _25_75,
        _0_100
    };

    bool CalculatePushPriority(
        const SteeringComponent& steerCompA,
        const SteeringComponent& steerCompB,
        EBumpRatio& outRatio)
    {
        bool sameTeam = steerCompA.Team == steerCompB.Team;
        bool aIsMoving = steerCompA.Mode == ESteerMode::Move;
        bool bIsMoving = steerCompB.Mode == ESteerMode::Move;

        uint8 pushPriAllyA = steerCompA.PushPriorityAlly;
        uint8 pushPriAllyB = steerCompB.PushPriorityAlly;

        if (sameTeam)
        {
            if (aIsMoving && pushPriAllyA >= pushPriAllyB && steerCompB.Mode != ESteerMode::Hold)
            {
                outRatio = EBumpRatio::_0_100;
                return true;
            }
            if (bIsMoving && pushPriAllyB >= pushPriAllyA && steerCompA.Mode != ESteerMode::Hold)
            {
                outRatio = EBumpRatio::_100_0;
                return true;
            }
        }

        return false;
    }

    EBumpRatio CalculateBumpRatio(
        EntityId entityA,
        const SteeringComponent& steerCompA,
        EntityId entityB,
        const SteeringComponent& steerCompB)
    {
        bool aIsMoving = steerCompA.Mode == ESteerMode::Move;
        bool bIsMoving = steerCompB.Mode == ESteerMode::Move;

        bool aIsHolding = steerCompA.Mode == ESteerMode::Hold;
        bool bIsHolding = steerCompB.Mode == ESteerMode::Hold;

        bool aIsFollowingB = steerCompA.GoalEntity == entityB;
        bool bIsFollowingA = steerCompB.GoalEntity == entityA;

        bool aIsFollowed = HasAnyFlags(steerCompA.Flags, ESteerFlags::Followed);
        bool bIsFollowed = HasAnyFlags(steerCompB.Flags, ESteerFlags::Followed);

        bool aIsBumped = HasAnyFlags(steerCompA.Flags, ESteerFlags::Bumped);
        bool bIsBumped = HasAnyFlags(steerCompB.Flags, ESteerFlags::Bumped);

        // Entities following another entity take all the bump.
        if (aIsFollowingB && !bIsFollowingA && !aIsHolding)
        {
            return EBumpRatio::_100_0;
        }
        if (!aIsFollowingB && bIsFollowingA && !bIsHolding)
        {
            return EBumpRatio::_0_100;
        }

        EBumpRatio pushRatio;
        if (CalculatePushPriority(steerCompA, steerCompB, pushRatio))
        {
            return pushRatio;
        }

        // Can't bump a holding entity.
        if (aIsHolding && !bIsHolding)
        {
            return EBumpRatio::_0_100;
        }
        if (!aIsHolding && bIsHolding)
        {
            return EBumpRatio::_100_0;
        }

        if (steerCompA.Team != steerCompB.Team)
        {
            if (!aIsMoving && aIsHolding && !aIsBumped)
            {
                return EBumpRatio::_0_100;
            }
            if (!bIsMoving && bIsHolding && !bIsBumped)
            {
                return EBumpRatio::_100_0;
            }
        }

        // Bump goes to the one not being followed.
        if (aIsFollowed && !bIsFollowed)
        {
            return EBumpRatio::_0_100;
        }
        if (!aIsFollowed && bIsFollowed)
        {
            return EBumpRatio::_100_0;
        }

        if (aIsMoving && !bIsMoving)
        {
            return bIsBumped ? EBumpRatio::_25_75 : EBumpRatio::_0_100;
        }
        if (!aIsMoving && bIsMoving)
        {
            return aIsBumped ? EBumpRatio::_75_25 : EBumpRatio::_100_0;
        }
        if (aIsMoving && bIsMoving)
        {
            return EBumpRatio::_50_50;
        }

        if (aIsBumped || !bIsBumped)
        {
            return EBumpRatio::_25_75;
        }
        if (!aIsBumped || bIsBumped)
        {
            return EBumpRatio::_75_25;
        }

        return EBumpRatio::_50_50;
    }

    struct SteeringJob : IJob<TransformComponent&, SteeringComponent&>
    {
        DeltaTime DeltaTime;

        bool DrawDebug = false;
        bool MoveTowardsGoal;
        Distance ArrivalThreshold;

        Distance DensityScalar;
        Distance DensityRadiusScalar;
        Distance AvoidanceScalar;
        Distance AvoidanceRadiusScalar;

        int32 MaxLookAheadSteps = 20;

        thread_local static std::vector<const SortedEntity*> EntityQueryStore;

        const char* GetName() const override { return "Steering.Steering"; }

        void Execute(
            WorldConstRef world,
            EntityId entityId,
            CommandBuffer& cb,
            TransformComponent& transformComp,
            SteeringComponent& steerComp) override
        {
            if (HasAnyFlags(steerComp.Flags, ESteerFlags::Attached, ESteerFlags::Hidden))
            {
                steerComp.PreviousPos = transformComp.Transform.Position;
                return;
            }

            if (steerComp.Mode == ESteerMode::Idle)
            {
                UpdateIdleEntity(transformComp, steerComp);
            }
            else if (steerComp.Mode == ESteerMode::Turn)
            {
                UpdateTurningEntity(transformComp, steerComp);
            }
            else if (steerComp.Mode == ESteerMode::Move)
            {
                UpdateMovingEntity(world, entityId, cb, transformComp, steerComp);
            }
            else if (steerComp.Mode == ESteerMode::Hold)
            {
                UpdateHoldingEntity(transformComp, steerComp);
            }
        }

        void UpdateHoldingEntity(TransformComponent& transformComp, const SteeringComponent& steerComp)
        {
            if (HasNoneFlags(steerComp.Flags, ESteerFlags::LockFacing) &&
                !Vec2::Equals(transformComp.Transform.Position, steerComp.GoalPos))
            {
                Vec2 heading = steerComp.GoalPos - transformComp.Transform.Position;
                transformComp.Transform.Rotation = CalculateFacingAngle(transformComp, steerComp, heading, false);
            }
        }

        void UpdateIdleEntity(TransformComponent& transformComp, SteeringComponent& steerComp)
        {
            PHX_ASSERT(steerComp.Mode == ESteerMode::Idle);

            const Vec2& currPos = transformComp.Transform.Position;
            Vec2 vel = currPos - steerComp.PreviousPos;

            while (vel.Length() > steerComp.MaxSpeed * DeltaTime)
            {
                vel /= 2;
            }

            vel = vel / 8 * 7;

            steerComp.Velocity = vel;

            if (!HasAnyFlags(steerComp.Flags, ESteerFlags::LockFacing))
            {
                transformComp.Transform.Rotation = CalculateFacingAngle(transformComp, steerComp, vel, false);
            }
        }

        void UpdateTurningEntity(TransformComponent& transformComp, const SteeringComponent& steerComp)
        {
            if (HasNoneFlags(steerComp.Flags, ESteerFlags::LockFacing) && 
                !Vec2::Equals(transformComp.Transform.Position, steerComp.GoalPos))
            {
                Vec2 heading = steerComp.GoalPos - transformComp.Transform.Position;
                transformComp.Transform.Rotation = CalculateFacingAngle(transformComp, steerComp, heading, false);
            }
        }

        void UpdateMovingEntity(
            WorldConstRef world,
            EntityId entityId,
            CommandBuffer& cb,
            TransformComponent& transformComp,
            SteeringComponent& steerComp)
        {
            const Vec2& currPos = transformComp.Transform.Position;
            Vec2 vel = currPos - steerComp.PreviousPos;

            auto vecToStep0 = steerComp.StepPos[0] - currPos;
            auto distToStep0 = vecToStep0.Length();

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

            if (!arrived && steerComp.SpreadingSlack > distToGoal)
            {
                arrived = true;
            }

            if (arrived)
            {
                if (distToStep0 <= steerComp.MaxSpeed)
                {
                    vel = vecToStep0;
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

            vel = CalculateAvoidance(world, entityId, transformComp, steerComp, vel, cb, EntityQueryStore);

            steerComp.Velocity = vel;

            if (!HasAnyFlags(steerComp.Flags, ESteerFlags::LockFacing))
            {
                transformComp.Transform.Rotation = CalculateFacingAngle(transformComp, steerComp, vel, true);
            }
        }

        static Distance CalculateDistanceToGoal(const SteeringComponent& steerComp, const Vec2& currPos)
        {
            Distance stepDistToGoal = 0;
            if (Vec2::Equals(steerComp.StepPos[0], steerComp.GoalPos))
            {
                stepDistToGoal += Vec2::Distance(steerComp.StepPos[0], currPos);
            }
            else if (Vec2::Equals(steerComp.StepPos[1], steerComp.GoalPos))
            {
                stepDistToGoal += Vec2::Distance(steerComp.StepPos[1], steerComp.StepPos[0]);
                stepDistToGoal += Vec2::Distance(steerComp.StepPos[0], currPos);
            }
            else
            {
                stepDistToGoal += Vec2::Distance(steerComp.GoalPos, steerComp.StepPos[1]);
                stepDistToGoal += Vec2::Distance(steerComp.StepPos[1], steerComp.StepPos[0]);
                stepDistToGoal += Vec2::Distance(steerComp.StepPos[0], currPos);
            }
            return stepDistToGoal;
        }

        Vec2 CalculateVelocity(const TransformComponent& transformComp, const SteeringComponent& steerComp) const
        {
            const Vec2& currPos = transformComp.Transform.Position;
            const Vec2& stepPos = steerComp.StepPos[0];
            Distance currSpeed = steerComp.Velocity.Length() / DeltaTime;
            Vec2 stepVec = stepPos - currPos;
            Vec2 stepDir = stepVec.Normalized();

            // Calculate deceleration
            if (steerComp.DecelerationTime > 0)
            {
                Distance stepDistToGoal = CalculateDistanceToGoal(steerComp, currPos);
                Distance requiredDist = (currSpeed * currSpeed) / (2 * (steerComp.MaxSpeed / steerComp.DecelerationTime));
                if (stepDistToGoal < requiredDist)
                {
                    Distance requiredDecel = -(currSpeed * currSpeed) / (2 * stepDistToGoal);
                    Distance newSpeed = currSpeed + requiredDecel * DeltaTime;
                    newSpeed = Max(newSpeed, 0);
                    return stepDir * newSpeed * DeltaTime;
                }
            }

            // Calculate acceleration
            if (steerComp.AccelerationTime > 0 && currSpeed < steerComp.MaxSpeed)
            {
                Distance requiredAccel = (steerComp.MaxSpeed - currSpeed) / steerComp.AccelerationTime;
                Distance newSpeed = currSpeed + requiredAccel * DeltaTime;
                newSpeed = Min(newSpeed, steerComp.MaxSpeed);
                return stepDir * newSpeed * DeltaTime;
            }

            return stepDir * steerComp.MaxSpeed * DeltaTime;
        }

        static bool CircleCast(
            const Vec2& currPosA,
            Distance radiusA,
            const Vec2& velA,
            const Vec2& currPosB,
            Distance radiusB,
            const Vec2& velB,
            int32 steps)
        {
            Vec2 nextPosA = currPosA + velA * steps;
            Vec2 nextPosB = currPosB + velB * steps;
            return Vec2::Distance(nextPosA, nextPosB) <= radiusA + radiusB;
        }

        uint32 QueryRelevantEntities(
            WorldConstRef world,
            EntityId entityId,
            const SteeringComponent& steerComp,
            const Vec2& center,
            Distance radius,
            const Vec2& moveDir,
            std::vector<const SortedEntity*>& entityQueryStore,
            TInlineLeaderboard<const SortedEntity*, Distance, MaxEntitiesPerQuery>& outClosestEntities) const
        {
            SteeringRangeQueryArgs queryArgs = { steerComp.CollisionMask, { entityId } };
            entityQueryStore.clear();
            FeatureSteering::QueryEntitiesInRange(world, center, radius, entityQueryStore, queryArgs);

            if (entityQueryStore.empty())
                return 0;

            for (const SortedEntity* other : entityQueryStore)
            {
                // If this entity is following the other entity then ignore it.
                if (steerComp.GoalEntity == other->EntityId)
                {
                    continue;
                }

                bool sameTeam = steerComp.Team == other->SteeringComponent->Team;

                // If the other entity is on the same team and is following this entity then ignore it.
                if (sameTeam && other->SteeringComponent->GoalEntity == entityId)
                {
                    continue;
                }

                EBumpRatio bumpRatio = CalculateBumpRatio(
                    entityId,
                    steerComp,
                    other->EntityId,
                    *other->SteeringComponent);

                // If the other entity is idle and on the same team then it can be pushed.
                if (other->SteeringComponent->Mode == ESteerMode::Idle && sameTeam && bumpRatio != EBumpRatio::_100_0)
                {
                    continue;
                }

                // Circle overlap by increasing the search radius by the entity radius
                const Vec2& otherPos = other->TransformComponent->Transform.Position;
                Distance dist = Vec2::Distance(otherPos, center);
                if (dist > radius + other->SteeringComponent->OuterRadius)
                {
                    continue;
                }

                constexpr Value cKeepUpScale = 0.95f;
                bool canKeepUp = other->SteeringComponent->MaxSpeed >= steerComp.MaxSpeed * cKeepUpScale;

                // If the other entity is on the same team, is moving in the same direction and has the capability to keep up then ignore it.
                if (sameTeam && canKeepUp)
                {
                    Vec2 otherMoveDir = other->SteeringComponent->StepPos[0] - otherPos;

                    // The other entity is barely moving, if at all, ignore it and rely on collision.
                    if ((Vec2::SqrxQ(otherMoveDir) >> 16) == 0)
                    {
                        continue;
                    }

                    if (Vec2::Dot(moveDir, otherMoveDir) > 0)
                    {
                        Angle angle = Abs(AngleDelta(otherMoveDir.AsRadians(), moveDir.AsRadians()));
                        if (angle <= Deg2Rad(60.0))
                        {
                            continue;
                        }
                    }
                }

                outClosestEntities.Add(other, dist);
            }

            return outClosestEntities.GetNum();
        }

        // Calculate a new velocity given a desired velocity that avoids dynamic and static obstacles.
        Vec2 CalculateAvoidance(
            WorldConstRef world,
            EntityId entityId,
            const TransformComponent& transformComp,
            SteeringComponent& steerComp,
            const Vec2& desiredVel,
            CommandBuffer& cb,
            std::vector<const SortedEntity*>& entityQueryStore) const
        {
            Distance desiredSpeed = desiredVel.Length();
            if (desiredSpeed == 0)
                return desiredVel;

            const Vec2& currPos = transformComp.Transform.Position;
            Vec2 desiredDir = desiredVel.Normalized();

            if (steerComp.Mode == ESteerMode::Move)
            {
                Vec2 stepVec = steerComp.StepPos[0] - currPos;
                if (Vec2::Dot(desiredDir, stepVec) <= 0)
                    return desiredVel;
            }

            Distance maxLookAheadDist = Vec2::Distance(currPos, steerComp.StepPos[0]);
            int32 lookAheadSteps = (int32)(maxLookAheadDist / desiredSpeed);
            lookAheadSteps = Min(lookAheadSteps, MaxLookAheadSteps);
            Distance searchRadius = desiredSpeed * lookAheadSteps + steerComp.OuterRadius;
            Vec2 searchCenter = currPos + desiredDir * (searchRadius / 2);

            if (DrawDebug)
            {
                cb.Append<Debug::Commands::DrawCircle>(searchCenter, searchRadius, Color::White);
            }

            TInlineLeaderboard<const SortedEntity*, Distance, MaxEntitiesPerQuery> closestEntities;
            QueryRelevantEntities(world, entityId, steerComp, searchCenter, searchRadius, desiredDir, entityQueryStore, closestEntities);

            if (closestEntities.IsEmpty())
                return desiredVel;

            constexpr Angle searchAngleDelta = Deg2Rad(5.0);
            constexpr Angle maxSearchAngle = Rad180;
            int32 bestStep = INT32_MIN;
            Vec2 bestNewVel = desiredVel;
            ESteerFlags bestSteerDir = ESteerFlags::None;
            bool foundAnyCollision = false;
            uint32 foundCollisions = 0;

            // Sweep angles 0→180 (both left and right), pick minimum rotation with no collision,
            // or failing that, the rotation that pushes the earliest collision furthest out.
            Angle angle = 0;
            for (; angle < maxSearchAngle; angle += searchAngleDelta)
            {
                ESteerFlags steerDirs[2];
                uint32 steerCount;
                Color colors[2] = { Color::Green, Color::Blue };

                if (angle == 0)
                {
                    steerDirs[0] = ESteerFlags::None;
                    colors[0] = Color::Red;
                    steerCount = 1;
                }
                else if (HasAnyFlags(steerComp.Flags, ESteerFlags::SteeringLeft, ESteerFlags::SteeringRight))
                {
                    // The entity can't be steering in both directions...
                    PHX_ASSERT(!HasAllFlags(steerComp.Flags, ESteerFlags::SteeringLeft, ESteerFlags::SteeringRight));

                    // Isolate steering direction
                    steerDirs[0] = HasAnyFlags(steerComp.Flags, ESteerFlags::SteeringLeft) ? ESteerFlags::SteeringLeft : ESteerFlags::SteeringRight;
                    steerCount = 1;
                }
                else
                {
                    steerDirs[0] = ESteerFlags::SteeringLeft;
                    steerDirs[1] = ESteerFlags::SteeringRight;
                    steerCount = 2;
                }

                uint32 foundCollisionsAtAngle = 0;

                for (uint32 steerIdx = 0; steerIdx < steerCount; ++steerIdx)
                {
                    ESteerFlags steerDir = steerDirs[steerIdx];
                    Angle steerAngle = steerDir == ESteerFlags::SteeringRight ? -angle : angle;
                    Vec2 rotatedVel = desiredVel.Rotate(steerAngle);

                    // TODO (jfarris): trace world and reduce lookAheadSteps accordingly.

                    int32 minCollisionStep = lookAheadSteps;
                    for (auto && [other, _] : closestEntities)
                    {
                        const Vec2& otherPos = other->TransformComponent->Transform.Position;
                        const Distance& otherRadius = other->SteeringComponent->OuterRadius;
                        Vec2 otherVel = otherPos - other->SteeringComponent->PreviousPos;

                        for (int32 step = 1; step <= lookAheadSteps; ++step)
                        {
                            if (CircleCast(
                                currPos, steerComp.OuterRadius, rotatedVel,
                                otherPos, otherRadius, otherVel,
                                step))
                            {
                                if (DrawDebug)
                                {
                                    cb.Append<Debug::Commands::DrawCircle>(otherPos, otherRadius, Color::White);
                                    cb.Append<Debug::Commands::DrawCircle>(currPos + rotatedVel * step, 0.1);
                                }

                                foundCollisionsAtAngle |= 1 << steerIdx;
                                foundCollisions |= 1 << steerIdx;
                                minCollisionStep = Min(minCollisionStep, step);
                                break;
                            }
                        }
                    }

                    bool foundCollisionInDir = foundCollisions & (1 << steerIdx);

                    // Update the best angle and step
                    if (!foundCollisionInDir || minCollisionStep > bestStep)
                    {
                        bestStep = minCollisionStep;
                        bestNewVel = rotatedVel;
                        bestSteerDir = steerDir;
                    }

                    // We didn't find a collision in this steering direction, so this must be the best direction.
                    if (!foundCollisionInDir)
                    {
                        break;
                    }

                    if (DrawDebug)
                    {
                        cb.Append<Debug::Commands::DrawRay>(currPos, rotatedVel * minCollisionStep, colors[steerIdx]);
                    }
                }

                foundAnyCollision = foundCollisionsAtAngle != 0;

                // Exit when we do not find a collision in either steering direction
                if (!foundAnyCollision)
                {
                    break;
                }
            }

            ClearFlagRef(steerComp.Flags, ESteerFlags::SteeringLeft);
            ClearFlagRef(steerComp.Flags, ESteerFlags::SteeringRight);
            SetFlagRef(steerComp.Flags, bestSteerDir);

            if (DrawDebug)
            {
                cb.Append<Debug::Commands::DrawRay>(currPos, bestNewVel.Normalized(), Color::Red);
            }

            // Clamp steering angle to avoid turning too sharply when steering to avoid a collision
            if (foundAnyCollision)
            {
                Angle turnRate = steerComp.TurnRateMoving > 0 ? (Pi / steerComp.TurnRateMoving * DeltaTime) : 0.1;
                Angle maxTurnRate = Max(turnRate, 0.1);
                Vec2 minSteerVec = Vec2::FromPolar(transformComp.Transform.Rotation - maxTurnRate, desiredSpeed);
                Vec2 maxSteerVec = Vec2::FromPolar(transformComp.Transform.Rotation + maxTurnRate, desiredSpeed);
                if (Vec2::Cross(bestNewVel, maxSteerVec) < 0)
                {
                    bestNewVel = maxSteerVec;
                }
                else if (Vec2::Cross(minSteerVec, bestNewVel) < 0)
                {
                    bestNewVel = minSteerVec;
                }
            }

            if (DrawDebug)
            {
                cb.Append<Debug::Commands::DrawRay>(currPos, bestNewVel.Normalized(), Color::Yellow);
            }

            return bestNewVel;
        }

        Angle CalculateFacingAngle(
            const TransformComponent& transformComp,
            const SteeringComponent& steerComp,
            const Vec2& heading,
            bool isMoving) const
        {
            Distance len = heading.Length();
            Distance thresh = Distance(Q32(128));
            if (len <= thresh)
            {
                return transformComp.Transform.Rotation;
            }

            Angle angle = Vec2::Equals(heading, Vec2::Zero) ? transformComp.Transform.Rotation : heading.AsRadians();
            Angle delta = AngleDelta(angle, transformComp.Transform.Rotation);
            Angle absDelta = Abs(delta);

            if (absDelta <= Angle::Epsilon)
            {
                return angle;
            }

            Angle turnRate = isMoving ? steerComp.TurnRateMoving : steerComp.TurnRateIdle;
            if (turnRate == 0)
            {
                return angle;
            }

            Angle turnRateDt = Pi / turnRate * DeltaTime;
            Angle newAngle = angle;

            if (absDelta >= turnRateDt)
            {
                if (delta > 0)
                {
                    newAngle = transformComp.Transform.Rotation + turnRateDt;
                }
                else
                {
                    newAngle = transformComp.Transform.Rotation - turnRateDt;
                }
            }

            return Cordic::AngleShift(newAngle);
        }
    };

    thread_local std::vector<const SortedEntity*> SteeringJob::EntityQueryStore;

    static void ClampVelocity(TransformComponent& transformComp, const SteeringComponent& steerComp)
    {
        Vec2 v = transformComp.Transform.Position - steerComp.PreviousPos;
        Distance d = v.Length();
        if (d >= steerComp.MaxSpeed)
        {
            d = steerComp.MaxSpeed;
            transformComp.Transform.Position = steerComp.PreviousPos + v.Normalized() * d;
        }
    }

    struct IntegrateJob : IJob<TransformComponent&, SteeringComponent&>
    {
        bool DrawDebug = false;
        Value SlackIncreaseRate;
        Value SlackIncreaseRateFast;
        Value SlackRateDivisor;
        Value SlackRateDivisorSlow;
        Value MaxSlack;

        thread_local static std::vector<const SortedEntity*> EntityQueryStore;

        const char* GetName() const override { return "Steering.Integrate"; }

        void Execute(
            WorldConstRef world,
            EntityId entityId,
            CommandBuffer& cb,
            TransformComponent& transformComp,
            SteeringComponent& steerComp) override
        {
            steerComp.PreviousPos = transformComp.Transform.Position;
            transformComp.Transform.Position += steerComp.Velocity;

            ClampVelocity(transformComp, steerComp);

            if (steerComp.Mode == ESteerMode::Move)
            {
                UpdateSlack(world, entityId, transformComp, steerComp, cb, EntityQueryStore);
            }

            if (!Vec2::Equals(steerComp.PreviousPos, transformComp.Transform.Position))
            {
                SetFlagRef(steerComp.Flags, ESteerFlags::Active);
            }

            CollideWithWorld(world, transformComp, steerComp);
        }

        void UpdateSlack(
            WorldConstRef world,
            EntityId entityId,
            const TransformComponent& transformComp,
            SteeringComponent& steerComp,
            CommandBuffer& cb,
            std::vector<const SortedEntity*>& entityQueryStore) const
        {
            if (steerComp.Mode != ESteerMode::Move)
            {
                steerComp.Slack = 0;
                return;
            }

            Vec2 currPos = transformComp.Transform.Position;
            Vec2 dest = steerComp.StepPos[0];
            Distance currToDest = Vec2::Distance(dest, currPos);
            Distance bestToDest = Vec2::Distance(dest, steerComp.BestPos);

            if (currToDest < bestToDest || Vec2::Equals(steerComp.BestPos, Vec2::Max))
            {
                steerComp.BestPos = currPos;
                steerComp.Slack /= 2;
                if (steerComp.Slack < steerComp.InnerRadius)
                {
                    steerComp.Slack = 0;
                }
                return;
            }

            SteeringRangeQueryArgs queryArgs = { steerComp.CollisionMask, { entityId } };
            entityQueryStore.clear();
            FeatureSteering::QueryEntitiesInRange(world, steerComp.GoalPos, steerComp.Slack, entityQueryStore, queryArgs);

            if (entityQueryStore.empty())
            {
                return;
            }

            uint64 crowdedness = 0;
            for (const SortedEntity* entity : entityQueryStore)
            {
                crowdedness += SqrxQ(entity->SteeringComponent->OuterRadius);
            }

            Value increaseRate = 1.0;
            if (steerComp.Slack > 0)
            {
                uint64 slackArea = SqrxQ(steerComp.Slack);
                crowdedness = std::min(crowdedness, slackArea);
                Value t = Value::QT(crowdedness / slackArea);
                increaseRate = Lerp01<Value>(SlackIncreaseRate, SlackIncreaseRateFast, t);
            }

            if (HasAnyFlags(steerComp.Flags, ESteerFlags::SteeringLeft, ESteerFlags::SteeringRight))
            {
                steerComp.Slack += steerComp.InnerRadius * increaseRate / SlackRateDivisorSlow;
            }
            else
            {
                steerComp.Slack += steerComp.InnerRadius * increaseRate / SlackRateDivisor;
            }

            if (steerComp.Slack > MaxSlack)
            {
                steerComp.Slack = MaxSlack;
            }

            if (DrawDebug && steerComp.Slack > 0)
            {
                cb.Append<Debug::Commands::DrawCircle>(currPos, steerComp.Slack, Color::Yellow);
            }
        }

        void CollideWithWorld(
            WorldConstRef world,
            const TransformComponent& transformComp,
            const SteeringComponent& steerComp)
        {
        }
    };

    thread_local std::vector<const SortedEntity*> IntegrateJob::EntityQueryStore;

    void SortEntities(FeatureSteeringScratchBlock& scratch)
    {
        for (SortedEntity& entity : scratch.SortedEntities)
        {
            entity.ZCode = ToMortonCode(entity.TransformComponent->Transform.Position);
        }

        std::ranges::sort(scratch.SortedEntities, [](const SortedEntity& a, const SortedEntity& b)
        {
            return a.ZCode < b.ZCode;
        });
    }

    struct CollisionTask : ITask
    {
        Distance AdditionalSearchRange = 3;

        const char* GetName() const override { return "Steering.Collision"; }

        void Run(WorldConstRef world, CommandBuffer& cb) override
        {
            auto& scratch = *const_cast<FeatureSteeringScratchBlock*>(world.GetBlock<FeatureSteeringScratchBlock>());

            // Resort entities now that the integrate step has moved their positions
            // Also populate the active entities list
            {
                SortEntities(scratch);

                scratch.ActiveEntities.Reset();
                for (SortedEntity& entity : scratch.SortedEntities)
                {
                    // Only touch entities that have potentially moved positions
                    if (HasAnyFlags(entity.SteeringComponent->Flags, ESteerFlags::Active))
                    {
                        scratch.ActiveEntities.PushBack(&entity);
                    }
                }
            }

            TMortonCodeRangeArray ranges;
            const SortedEntity* neighbors[64];
            uint32 neighborsCount = 0;

            for (uint32 i = 0; i < 2; ++i)
            {
                const uint32 count = scratch.ActiveEntities.GetNum();
                for (uint32 idx = 0; idx < count; ++idx)
                {
                    const SortedEntity& entityA = *scratch.ActiveEntities[idx];
                    TransformComponent& transformCompA = *entityA.TransformComponent;
                    SteeringComponent& steerCompA = *entityA.SteeringComponent;

                    // Query for overlapping morton ranges
                    {
                        PHX_PROFILE_ZONE_SCOPED_N("OverlapQuery");

                        Distance range = steerCompA.OuterRadius + AdditionalSearchRange;
                        MortonCodeAABB aabb = ToMortonCodeAABB(transformCompA.Transform.Position, range);

                        ranges.clear();
                        MortonCodeQuery(aabb, ranges);

                        neighborsCount = 0;
                        ForEachInMortonCodeRanges<SortedEntity, &SortedEntity::ZCode>(
                            scratch.SortedEntities,
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

                    for (uint32 neighborIdx = 0; neighborIdx < neighborsCount; ++neighborIdx)
                    {
                        const SortedEntity* entityB = neighbors[neighborIdx];
                        TransformComponent& transformCompB = *entityB->TransformComponent;
                        SteeringComponent& steerCompB = *entityB->SteeringComponent;

                        Vec2& currPosA = transformCompA.Transform.Position;
                        const Vec2& prevPosA = steerCompA.PreviousPos;
                        const Vec2 velA = currPosA - prevPosA;

                        Vec2& currPosB = transformCompB.Transform.Position;
                        const Vec2& prevPosB = steerCompB.PreviousPos;
                        const Vec2 velB = currPosB - prevPosB;

                        auto radius = steerCompA.OuterRadius + steerCompB.OuterRadius;
                        auto rr = SqrxQ(radius);

                        auto d = currPosB - currPosA;
                        auto dd = Vec2::SqrxQ(d);

                        if (dd >= rr)
                        {
                            continue;
                        }

                        if (dd == 0)
                        {
                            d = Vec2(radius, 0);
                            // dd = rr;
                        }

                        EBumpRatio ratio = CalculateBumpRatio(entityA.EntityId, steerCompA, entityB->EntityId, steerCompB);

                        auto n = d.Normalized();
                        auto r = n * radius;
                        auto separation = r - d;

                        if (ratio == EBumpRatio::_100_0)
                        {
                            if (HasNoneFlags(steerCompA.Flags, ESteerFlags::Bumped) &&
                                steerCompA.Mode == ESteerMode::Idle &&
                                steerCompB.Mode != ESteerMode::Idle)
                            {
                                auto relVel = velB - velA;
                                relVel = Vec2::Cross(d, relVel) < 0 ? relVel.Rotate(-Rad90) : relVel.Rotate(Rad90);
                                separation += relVel * 2;
                                cb.Append<Debug::Commands::DrawRay>(currPosA, -separation, Color::Yellow);
                            }

                            currPosA -= separation;
                            SetFlagRef(steerCompA.Flags, ESteerFlags::Bumped);
                            SpreadSlack(transformCompA, steerCompA, transformCompB, steerCompB);
                        }
                        else if (ratio == EBumpRatio::_75_25)
                        {
                            currPosA -= separation * 3/4;
                            currPosB += separation * 1/4;
                            SetFlagRef(steerCompA.Flags, ESteerFlags::Bumped);
                            SetFlagRef(steerCompB.Flags, ESteerFlags::Bumped);
                            SpreadSlack(transformCompA, steerCompA, transformCompB, steerCompB);
                        }
                        else if (ratio == EBumpRatio::_50_50)
                        {
                            currPosA -= separation * 1/2;
                            currPosB += separation * 1/2;
                            SetFlagRef(steerCompA.Flags, ESteerFlags::Bumped);
                            SetFlagRef(steerCompB.Flags, ESteerFlags::Bumped);
                            SpreadSlack(transformCompA, steerCompA, transformCompB, steerCompB);
                        }
                        else if (ratio == EBumpRatio::_25_75)
                        {
                            currPosA -= separation * 1/4;
                            currPosB += separation * 3/4;
                            SetFlagRef(steerCompA.Flags, ESteerFlags::Bumped);
                            SetFlagRef(steerCompB.Flags, ESteerFlags::Bumped);
                            SpreadSlack(transformCompA, steerCompA, transformCompB, steerCompB);
                        }
                        else if (ratio == EBumpRatio::_0_100)
                        {
                            if (HasNoneFlags(steerCompB.Flags, ESteerFlags::Bumped) &&
                                steerCompA.Mode != ESteerMode::Idle &&
                                steerCompB.Mode == ESteerMode::Idle)
                            {
                                auto relVel = velA - velB;
                                relVel = Vec2::Cross(relVel, d) < 0 ? relVel.Rotate(-Rad90) : relVel.Rotate(Rad90);
                                separation += relVel * 2;
                                cb.Append<Debug::Commands::DrawRay>(currPosB, separation, Color::Green);
                            }

                            currPosB += separation;
                            SetFlagRef(steerCompB.Flags, ESteerFlags::Bumped);
                            SpreadSlack(transformCompA, steerCompA, transformCompB, steerCompB);
                        }

                        // EntityB is activated by being pushed.
                        bool activateEntityB = ratio > EBumpRatio::_100_0;
                        if (activateEntityB && HasNoneFlags(steerCompB.Flags, ESteerFlags::Active))
                        {
                            SetFlagRef(steerCompB.Flags, ESteerFlags::Active);
                            scratch.ActiveEntities.PushBack(entityB);
                        }
                    }
                }
            }

            for (const SortedEntity* entity : scratch.ActiveEntities)
            {
                ClampVelocity(*entity->TransformComponent, *entity->SteeringComponent);
            }

            for (const SortedEntity& entity : scratch.SortedEntities)
            {
                ClearFlagRef(entity.SteeringComponent->Flags, ESteerFlags::Active);
                ClearFlagRef(entity.SteeringComponent->Flags, ESteerFlags::Bumped);
            }
        }

        static void SpreadSlack(
            const TransformComponent& transCompA,
            SteeringComponent& steerCompA,
            const TransformComponent& transCompB,
            SteeringComponent& steerCompB)
        {
            if (!Vec2::Equals(steerCompA.GoalPos, steerCompB.GoalPos))
            {
                return;
            }

            Distance distToGoalA = Vec2::Distance(transCompA.Transform.Position, steerCompA.GoalPos);
            Distance distToGoalB = Vec2::Distance(transCompB.Transform.Position, steerCompB.GoalPos);

            if (steerCompA.Mode == ESteerMode::Idle)
            {
                steerCompA.SpreadingSlack = Max(steerCompA.SpreadingSlack, distToGoalA);
            }

            if (steerCompB.Mode == ESteerMode::Idle)
            {
                steerCompB.SpreadingSlack = Max(steerCompB.SpreadingSlack, distToGoalB);
            }

            if (steerCompA.Mode == ESteerMode::Idle)
            {
                steerCompB.SpreadingSlack = Max(steerCompA.SpreadingSlack, steerCompB.SpreadingSlack);
            }

            if (steerCompB.Mode == ESteerMode::Idle)
            {
                steerCompA.SpreadingSlack = Max(steerCompA.SpreadingSlack, steerCompB.SpreadingSlack);
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
        auto resetSortedEntities = std::make_unique<SteeringDetail::ResetSortedEntitiesTask>();
        auto scatterSortedEntities = std::make_unique<SteeringDetail::ScatterSortedEntitiesJob>();
        auto compactSortedEntities = std::make_unique<SteeringDetail::CompactSortedEntitiesTask>();

        JobHandle hResetSortedEntities = FeatureECS::RegisterJob(world, std::move(resetSortedEntities), EJobPhase::PreUpdate);
        JobHandle hScatterSortedEntities = FeatureECS::RegisterJob(world, std::move(scatterSortedEntities), EJobPhase::PreUpdate);
        JobHandle hCompactSortedEntities = FeatureECS::RegisterJob(world, std::move(compactSortedEntities), EJobPhase::PreUpdate);

        FeatureECS::AddJobDependency(world, EJobPhase::PreUpdate, hScatterSortedEntities, hResetSortedEntities);
        FeatureECS::AddJobDependency(world, EJobPhase::PreUpdate, hCompactSortedEntities, hScatterSortedEntities);

        // Scatter must run after ECS sort so SortedEntityIndex is populated
        JobHandle hECSSort = FeatureECS::GetPreUpdateSortJobHandle(world);
        FeatureECS::AddJobDependency(world, EJobPhase::PreUpdate, hScatterSortedEntities, hECSSort);
    }

    // --- Update scheduler: pathfinding → steering → collision ---
    {
        PathfindingJob = std::make_unique<SteeringDetail::PathfindingJob>();
        SteeringJob = std::make_unique<SteeringDetail::SteeringJob>();
        IntegrateJob = std::make_unique<SteeringDetail::IntegrateJob>();
        auto collisionTask = std::make_unique<SteeringDetail::CollisionTask>();

        JobHandle hPathfinding = FeatureECS::RegisterJob(world, PathfindingJob.get(), EJobPhase::Update);
        JobHandle hSteering = FeatureECS::RegisterJob(world, SteeringJob.get(), EJobPhase::Update);
        JobHandle hIntegrate = FeatureECS::RegisterJob(world, IntegrateJob.get(), EJobPhase::Update);
        JobHandle hCollision = FeatureECS::RegisterJob(world, std::move(collisionTask), EJobPhase::Update);

        // Explicit ordering: pathfinding → steering → integrate → collision
        FeatureECS::AddJobDependency(world, EJobPhase::Update, hSteering, hPathfinding);
        FeatureECS::AddJobDependency(world, EJobPhase::Update, hIntegrate, hSteering);
        FeatureECS::AddJobDependency(world, EJobPhase::Update, hCollision, hIntegrate);
    }
}

void SteeringSystem::OnPreWorldUpdate(WorldRef world, const SystemUpdateArgs& /*args*/)
{
}

void SteeringSystem::OnWorldUpdate(WorldRef world, const SystemUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    PathfindingJob->ArrivalThreshold = ArrivalThreshold;

    SteeringJob->DeltaTime              = args.DeltaTime;
    SteeringJob->DrawDebug              = DrawDebug;
    SteeringJob->MoveTowardsGoal        = MoveTowardsGoal;
    SteeringJob->ArrivalThreshold       = ArrivalThreshold;
    SteeringJob->DensityScalar          = DensityScalar;
    SteeringJob->DensityRadiusScalar    = DensityRadiusScalar;
    SteeringJob->AvoidanceScalar        = AvoidanceScalar;
    SteeringJob->AvoidanceRadiusScalar  = AvoidanceRadiusScalar;
    SteeringJob->MaxLookAheadSteps      = MaxLookAheadSteps;

    IntegrateJob->DrawDebug             = DrawDebug;
    IntegrateJob->SlackIncreaseRate     = SlackIncreaseRate;
    IntegrateJob->SlackIncreaseRateFast = SlackIncreaseRateFast;
    IntegrateJob->SlackRateDivisor      = SlackRateDivisor;
    IntegrateJob->SlackRateDivisorSlow  = SlackRateDivisorSlow;
    IntegrateJob->MaxSlack              = MaxSlack;
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
