
#include "Phoenix.Sim.Physics/PhysicsSystem.h"
#include "Phoenix/Reflection/Registration.h"

#include "Phoenix/Color.h"
#include "Phoenix.Sim.Debug/Debug.h"
#include "Phoenix/Flags.h"
#include "Phoenix/MortonCode.h"
#include "Phoenix/Profiling.h"
#include "Phoenix.Sim/WorldTaskQueue.h"
#include "Phoenix.Sim.ECS/FeatureECS.h"
#include "Phoenix.Sim.ECS/SystemJob.h"
#include "Phoenix.Sim.Physics/BodyComponent.h"
#include "Phoenix.Sim.Physics/FeaturePhysics.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::Physics;

constexpr uint8 SLEEP_TIMER = 1;

namespace Phoenix::Physics::PhysicsSystemDetail
{
    struct ResetScratchBlockTask : ITask
    {
        const char* GetName() const override { return "Physics.ResetScratchBlockJob"; }

        void Run(WorldConstRef world, CommandBuffer& /*cb*/) override
        {
            auto* ecsScratch = world.GetBlock<FeatureECSScratchBlock>();
            auto* physicsScratch = const_cast<FeaturePhysicsScratchBlock*>(world.GetBlock<FeaturePhysicsScratchBlock>());

            // Invalidate scatter slots up to the high-water mark from the previous frame.
            // PrevSortedEntityCount is initialized to full capacity so frame 0 is safe.
            // Scatter only writes to positions 0..ecsCount-1, so this covers exactly the live range.
            std::memset(physicsScratch->SortedEntities.GetData(), 0xFF,
                        ecsScratch->PrevSortedEntityCount * sizeof(EntityBody));

            physicsScratch->SortedEntityCount = 0;
            physicsScratch->ContactPairs.Reset();
            physicsScratch->ContactPairsCount = 0;
        }
    };

    // Scatter each physics entity directly into its ECS-sorted position.
    // Requires SortedEntityIndex to be populated by the ECS sort task.
    // Uses read-only component access so it can run in parallel with other read-only jobs.
    struct ScatterSortedEntitiesJob : IJob<const TransformComponent&, const BodyComponent&>
    {
        const FeatureECSScratchBlock*   EcsScratch     = nullptr;
        FeaturePhysicsScratchBlock*     PhysicsScratch = nullptr;

        const char* GetName() const override { return "Physics.ScatterSortedEntitiesJob"; }

        void BeginBatch(WorldConstRef world, const JobBatch&, CommandBuffer&) override
        {
            EcsScratch     = world.GetBlock<FeatureECSScratchBlock>();
            PhysicsScratch = const_cast<FeaturePhysicsScratchBlock*>(world.GetBlock<FeaturePhysicsScratchBlock>());
        }

        void Execute(WorldConstRef /*world*/,
                     EntityId entityId,
                     CommandBuffer& /*cb*/,
                     const TransformComponent& transformComp,
                     const BodyComponent& bodyComp) override
        {
            const uint32 entityIdx = (entityId % EcsScratch->SortedEntityIndex.GetCapacity()) - 1;
            const uint32 sortedIdx = EcsScratch->SortedEntityIndex[entityIdx];
            if (sortedIdx != Index<uint32>::None)
            {
                PhysicsScratch->SortedEntities[sortedIdx] = {
                    .EntityId = entityId,
                    .TransformComponent = const_cast<TransformComponent*>(&transformComp),
                    .BodyComponent = const_cast<BodyComponent*>(&bodyComp),
                    .ZCode = transformComp.ZCode
                };
            }
        }
    };

    // Compact the scatter buffer into a dense sorted array.
    // Walks the ECS sorted range and collects positions that were written by ScatterSortedEntitiesJob.
    struct CompactSortedEntitiesTask : ITask
    {
        const char* GetName() const override { return "Physics.CompactSortedEntitiesTask"; }

        void Run(WorldConstRef world, CommandBuffer& /*cb*/) override
        {
            const auto* ecsScratch = world.GetBlock<FeatureECSScratchBlock>();
            auto* physicsScratch = const_cast<FeaturePhysicsScratchBlock*>(world.GetBlock<FeaturePhysicsScratchBlock>());
            const uint32 ecsCount = ecsScratch->SortedEntities.GetNum();
            EntityBody* data = physicsScratch->SortedEntities.GetData();

            // In-place compaction: valid entries are already in ZCode order (by construction).
            // Writes always go to count <= i so no entry is overwritten before being read.
            uint32 count = 0;
            for (uint32 i = 0; i < ecsCount; ++i)
            {
                if (data[i].EntityId != EntityId::Invalid)
                {
                    data[count++] = data[i];
                }
            }

            physicsScratch->SortedEntities.SetSize(count);
            physicsScratch->SortedEntityCount = count;
        }
    };

    struct IntegrateVelocitiesJob : IJob<BodyComponent&>
    {
        DeltaTime DeltaTime;

        const char* GetName() const override { return "Physics.IntegrateVelocitiesJob"; }

        void Execute(WorldConstRef /*world*/,
                     EntityId /*entityId*/,
                     CommandBuffer& /*cb*/,
                     BodyComponent& bodyComp) override
        {
            bodyComp.LinearVelocity += bodyComp.Force * bodyComp.InvMass * DeltaTime;

            if (bodyComp.MaxLinearVelocity > 0)
            {
                Distance vel = bodyComp.LinearVelocity.Length();
                vel = Min(vel, bodyComp.MaxLinearVelocity);
                bodyComp.LinearVelocity = bodyComp.LinearVelocity.Normalized() * vel;
            }

            bodyComp.Force = Vec2::Zero;
        }
    };

    struct CalculateContactPairsJob : IJob<TransformComponent&, BodyComponent&>
    {
        DeltaTime DeltaTime;
        FeaturePhysicsScratchBlock* ScratchBlock = nullptr;

        const char* GetName() const override { return "Physics.CalculateContactPairsJob"; }

        void BeginBatch(WorldConstRef world, const JobBatch&, CommandBuffer&) override
        {
            ScratchBlock = const_cast<FeaturePhysicsScratchBlock*>(world.GetBlock<FeaturePhysicsScratchBlock>());
        }

        void Execute(WorldConstRef /*world*/,
                     EntityId entityIdA,
                     CommandBuffer& /*cb*/,
                     TransformComponent& transformCompA,
                     BodyComponent& bodyCompA) override
        {
            if (!HasAnyFlags(bodyCompA.Flags, EBodyFlags::Awake))
                return;

            TMortonCodeRangeArray ranges;
            const EntityBody* overlappingBodies[PHX_PHS_MAX_CONTACTS_PER_ENTITY * 4];
            uint32 overlappingBodiesCount = 0;

            // Query for overlapping morton ranges
            {
                PHX_PROFILE_ZONE_SCOPED_N("OverlapQuery");

                Vec2 projectedPos = transformCompA.Transform.Position + bodyCompA.LinearVelocity * DeltaTime;
                MortonCodeAABB aabb = ToMortonCodeAABB(projectedPos, bodyCompA.Radius);

                ranges.clear();
                MortonCodeQuery(aabb, ranges);

                overlappingBodiesCount = 0;
                ForEachInMortonCodeRanges<EntityBody, &EntityBody::ZCode>(
                    ScratchBlock->SortedEntities,
                    ranges,
                    [&](const EntityBody& eb)
                    {
                        if (eb.EntityId == entityIdA)
                        {
                            return false;
                        }
                        if ((bodyCompA.CollisionMask & eb.BodyComponent->CollisionMask) == 0)
                        {
                            return false;
                        }
                        overlappingBodies[overlappingBodiesCount++] = &eb;
                        return overlappingBodiesCount == _countof(overlappingBodies);
                    });
            }

            for (uint32 i = 0; i < overlappingBodiesCount; ++i)
            {
                const EntityBody* entityBodyB = overlappingBodies[i];
                TransformComponent& transformCompB = *entityBodyB->TransformComponent;
                BodyComponent& bodyCompB = *entityBodyB->BodyComponent;

                Vec2 v = transformCompB.Transform.Position - transformCompA.Transform.Position;
                Distance vLen = v.Length();
                Distance rr = bodyCompA.Radius + bodyCompB.Radius;
                if (vLen > rr)
                {
                    continue;
                }

                entityid_t loId = Min(entityIdA, entityBodyB->EntityId);
                entityid_t hiId = Max(entityIdA, entityBodyB->EntityId);
                uint64 key = hiId; key = key << 32 | loId;

                uint32 contactIndex = ScratchBlock->ContactPairsCount.fetch_add(1);
                if (contactIndex >= ScratchBlock->ContactPairs.GetCapacity())
                {
                    break;
                }

                ContactPair& pair = ScratchBlock->ContactPairs[contactIndex];
                pair.Key = key;
                pair.TransformA = &transformCompA;
                pair.BodyA = &bodyCompA;
                pair.TransformB = &transformCompB;
                pair.BodyB = &bodyCompB;
            }
        }
    };

    void CalculateContactsTask(WorldRef world, uint32 startIndex, uint32 count, DeltaTime dt)
    {
        PHX_PROFILE_ZONE_SCOPED;

        FeaturePhysicsScratchBlock& scratchBlock = world.GetBufferUnsafe().GetBlockRef<FeaturePhysicsScratchBlock>();

        for (uint32 i = 0; i < count; ++i)
        {
            Contact& contact = scratchBlock.Contacts[startIndex + i];
            ContactPair& contactPair = scratchBlock.ContactPairs[contact.ContactPair];
            auto& bodyCompA = *contactPair.BodyA;
            auto& bodyCompB = *contactPair.BodyB;
            auto& transformCompA = *contactPair.TransformA;
            auto& transformCompB = *contactPair.TransformB;

            Vec2 v;
            if (Vec2::Equals(transformCompA.Transform.Position, transformCompB.Transform.Position))
            {
                v = Vec2::RandUnitVector();
                auto correction = OneDivBy(bodyCompA.InvMass + bodyCompB.InvMass);
                transformCompA.Transform.Position -= v * correction * 0.01f;
                transformCompB.Transform.Position += v * correction * 0.01f;
            }

            v = transformCompB.Transform.Position - transformCompA.Transform.Position;

            Distance vLen = v.Length();
            Distance rr = bodyCompA.Radius + bodyCompB.Radius;

            constexpr Value baum = 0.1f;
            const Value slop = 0.01f * rr;
            Distance d = rr - vLen;
            Value bias = -baum * Max(0, d - slop) / dt;

            contact.Normal = v.Normalized();
            contact.Bias = bias;
            contact.EffMass = OneDivBy(bodyCompA.InvMass + bodyCompB.InvMass);
        }
    }

    void ResolveContactPairsTask(WorldRef world)
    {
        PHX_PROFILE_ZONE_SCOPED;

        FeaturePhysicsScratchBlock& scratchBlock = world.GetBufferUnsafe().GetBlockRef<FeaturePhysicsScratchBlock>();

        scratchBlock.ContactPairs.SetSize(scratchBlock.ContactPairsCount);

        {
            PHX_PROFILE_ZONE_SCOPED_N("SortContactPairs");

            std::ranges::sort(
                scratchBlock.ContactPairs,
                [](const ContactPair& a, const ContactPair& b)
                {
                    return a.Key < b.Key;
                });
        }

        {
            PHX_PROFILE_ZONE_SCOPED_N("ResolveContactPairs");

            scratchBlock.Contacts.Reset();

            uint64 currContactPairKey = 0;
            uint32 numContacts = 0;
            for (uint32 i = 0; i < scratchBlock.ContactPairs.GetNum(); ++i)
            {
                const ContactPair& contactPair = scratchBlock.ContactPairs[i];

                if (contactPair.Key == currContactPairKey)
                    continue;

                currContactPairKey = contactPair.Key;

                Contact& contact = scratchBlock.Contacts[numContacts++];
                contact.ContactPair = i;

                if (numContacts == scratchBlock.Contacts.GetCapacity())
                    break;
            }

            scratchBlock.Contacts.SetSize(numContacts);
        }
    }

    void PGSTask(WorldRef world, uint32 startIndex, uint32 count, uint32 iter)
    {
        PHX_PROFILE_ZONE_SCOPED;
        PHX_PROFILE_ZONE_VALUE(iter);

        FeaturePhysicsScratchBlock& scratchBlock = world.GetBufferUnsafe().GetBlockRef<FeaturePhysicsScratchBlock>();

        for (uint32 i = 0; i < count; ++i)
        {
            Contact& contact = scratchBlock.Contacts[startIndex + i];
            ContactPair& contactPair = scratchBlock.ContactPairs[contact.ContactPair];

            Vec2 velA = Vec2::Zero;
            if (contactPair.BodyA)
                velA = contactPair.BodyA->LinearVelocity;

            Vec2 velB = Vec2::Zero;
            if (contactPair.BodyB)
                velB = contactPair.BodyB->LinearVelocity;

            Value relVel = Vec2::Dot(contact.Normal, velB - velA);

            Value lambda = -(relVel + contact.Bias) * contact.EffMass;

            Value oldImpulse = contact.Impulse;
            contact.Impulse = Max(oldImpulse + lambda, 0.0f);
            Value change = contact.Impulse - oldImpulse;

            Vec2 p = contact.Normal * change;
            if (contactPair.BodyA && !HasAnyFlags(contactPair.BodyA->Flags, EBodyFlags::Static))
                contactPair.BodyA->LinearVelocity -= p * contactPair.BodyA->InvMass;
            if (contactPair.BodyB && !HasAnyFlags(contactPair.BodyB->Flags, EBodyFlags::Static))
                contactPair.BodyB->LinearVelocity += p * contactPair.BodyB->InvMass;
        }
    }

    struct IntegrateJob : IJob<TransformComponent&, BodyComponent&>
    {
        DeltaTime DeltaTime;
        bool bAllowSleep = true;

        const char* GetName() const override { return "Physics.IntegrateJob"; }

        void Execute(WorldConstRef /*world*/,
                     EntityId /*entityId*/,
                     CommandBuffer& /*cb*/,
                     TransformComponent& transformComp,
                     BodyComponent& bodyComp) override
        {
            if (bodyComp.Movement == EBodyMovement::Attached)
            {
                transformComp.Transform.Rotation += 10.0f;
            }
            else
            {
                if (bAllowSleep)
                {
                    bool isMoving = bodyComp.LinearVelocity.Length() > Distance(1E-1);
                    if (isMoving)
                    {
                        bodyComp.SleepTimer = SLEEP_TIMER;
                        SetFlagRef(bodyComp.Flags, EBodyFlags::Awake, true);
                    }
                    else if (bodyComp.SleepTimer > 0)
                    {
                        --bodyComp.SleepTimer;
                        SetFlagRef(bodyComp.Flags, EBodyFlags::Awake, true);
                    }
                    else
                    {
                        SetFlagRef(bodyComp.Flags, EBodyFlags::Awake, false);
                    }
                }

                bodyComp.PreviousPos = transformComp.Transform.Position;
                transformComp.Transform.Position += bodyComp.LinearVelocity * DeltaTime;

                if (bodyComp.LinearDamping > 0)
                    bodyComp.LinearVelocity *= (1.0f - bodyComp.LinearDamping * DeltaTime);
            }
        }
    };

    void OverlapSeparationTask(WorldRef world, uint32 startIndex, uint32 count)
    {
        PHX_PROFILE_ZONE_SCOPED;

        FeaturePhysicsScratchBlock& scratchBlock = world.GetBufferUnsafe().GetBlockRef<FeaturePhysicsScratchBlock>();

        for (uint32 i = 0; i < count; ++i)
        {
            EntityBody& entityBody = scratchBlock.SortedEntities[startIndex + i];
            auto bodyCompA = entityBody.BodyComponent;
            auto transformCompA = entityBody.TransformComponent;

            for (const CollisionLine& line : scratchBlock.CollisionLines)
            {
                Vec2 v = PointToLine(line.Line, transformCompA->Transform.Position);
                Distance vLen = v.Length();
                if (vLen != 0.0f && vLen < bodyCompA->Radius)
                {
                    Vec2 n = -(v / vLen);
                    Vec2 d = n * (bodyCompA->Radius - vLen);
                    transformCompA->Transform.Position += d;
                    if (Vec2::Dot(bodyCompA->LinearVelocity, n) < 0)
                        bodyCompA->LinearVelocity = Vec2::Reflect(line.Line.GetDirection(), bodyCompA->LinearVelocity);
                    SetFlagRef(bodyCompA->Flags, EBodyFlags::Awake, true);
                }
            }
        }
    }

    void OverlapSeparationTask2(WorldRef world,
                                uint32 startIndex,
                                uint32 count,
                                Value penetrationThreshold,
                                Value penetrationCorrection)
    {
        PHX_PROFILE_ZONE_SCOPED;

        FeaturePhysicsScratchBlock& scratchBlock = world.GetBufferUnsafe().GetBlockRef<FeaturePhysicsScratchBlock>();

        for (uint32 i = 0; i < count; ++i)
        {
            Contact& contact = scratchBlock.Contacts[startIndex + i];
            ContactPair& contactPair = scratchBlock.ContactPairs[contact.ContactPair];

            Vec2 v = contactPair.TransformB->Transform.Position - contactPair.TransformA->Transform.Position;
            Distance d = v.Length();
            Distance rr = contactPair.BodyA->Radius + contactPair.BodyB->Radius;
            Distance pen = rr - d;
            if (pen > penetrationThreshold)
            {
                Value correction = penetrationCorrection * pen;
                contactPair.TransformA->Transform.Position -= contact.Normal * correction * contactPair.BodyA->InvMass / (contactPair.BodyA->InvMass + contactPair.BodyB->InvMass);
                contactPair.TransformB->Transform.Position += contact.Normal * correction * contactPair.BodyB->InvMass / (contactPair.BodyA->InvMass + contactPair.BodyB->InvMass);
                SetFlagRef(contactPair.BodyA->Flags, EBodyFlags::Awake, true);
                SetFlagRef(contactPair.BodyB->Flags, EBodyFlags::Awake, true);
            }
        }
    }
}

PhysicsSystem::PhysicsSystem()
    : IntegrateVelocitiesScheduler("Physics.IntegrateVelocities")
    , CalculateContactPairsScheduler("Physics.CalculateContactPairs")
    , IntegrateScheduler("Physics.Integrate")
{
}

// This is dumb but is required because IntegrateJob is only defined in the cpp.
PhysicsSystem::~PhysicsSystem() = default;

void PhysicsSystem::OnWorldInitialize(WorldRef world)
{
    const FeatureECSDynamicBlock& ecsBlock = world.GetBlockRef<FeatureECSDynamicBlock>();

    // --- Pre-update scheduler: reset → scatter (using ECS sorted index) → compact ---
    {
        auto reset   = std::make_unique<PhysicsSystemDetail::ResetScratchBlockTask>();
        auto scatter = std::make_unique<PhysicsSystemDetail::ScatterSortedEntitiesJob>();
        auto compact = std::make_unique<PhysicsSystemDetail::CompactSortedEntitiesTask>();

        JobHandle hReset   = FeatureECS::RegisterJob(world, std::move(reset),   EJobPhase::PreUpdate);
        JobHandle hScatter = FeatureECS::RegisterJob(world, std::move(scatter), EJobPhase::PreUpdate);
        JobHandle hCompact = FeatureECS::RegisterJob(world, std::move(compact), EJobPhase::PreUpdate);

        FeatureECS::AddJobDependency(world, EJobPhase::PreUpdate, hScatter, hReset);
        FeatureECS::AddJobDependency(world, EJobPhase::PreUpdate, hCompact, hScatter);

        // Scatter reads SortedEntityIndex which is written by the ECS sort task.
        JobHandle hECSSort = FeatureECS::GetPreUpdateSortJobHandle(world);
        FeatureECS::AddJobDependency(world, EJobPhase::PreUpdate, hScatter, hECSSort);
    }

    // --- IntegrateVelocities scheduler ---
    IntegrateVelocitiesJob = std::make_unique<PhysicsSystemDetail::IntegrateVelocitiesJob>();
    IntegrateVelocitiesScheduler.RegisterJob(IntegrateVelocitiesJob.get());
    IntegrateVelocitiesScheduler.Build(ecsBlock.ArchetypeManager);

    // --- CalculateContactPairs scheduler ---
    CalculateContactPairsJob = std::make_unique<PhysicsSystemDetail::CalculateContactPairsJob>();
    CalculateContactPairsScheduler.RegisterJob(CalculateContactPairsJob.get());
    CalculateContactPairsScheduler.Build(ecsBlock.ArchetypeManager);

    // --- Integrate scheduler ---
    IntegrateJob = std::make_unique<PhysicsSystemDetail::IntegrateJob>();
    IntegrateScheduler.RegisterJob(IntegrateJob.get());
    IntegrateScheduler.Build(ecsBlock.ArchetypeManager);

    // Register schedulers for debug visualization
    FeatureECS::RegisterScheduler(world, IntegrateVelocitiesScheduler);
    FeatureECS::RegisterScheduler(world, CalculateContactPairsScheduler);
    FeatureECS::RegisterScheduler(world, IntegrateScheduler);
}

void PhysicsSystem::OnPreWorldUpdate(WorldRef world, const SystemUpdateArgs& args)
{
}

void PhysicsSystem::OnWorldUpdate(WorldRef world, const SystemUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;
}

void PhysicsSystem::OnPostWorldUpdate(WorldRef world, const SystemUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    const DeltaTime dt = args.DeltaTime;
    FeaturePhysicsScratchBlock& scratchBlock = world.GetBlockRef<FeaturePhysicsScratchBlock>();

    // Integrate velocities (once, before the iteration loop)
    IntegrateVelocitiesJob->DeltaTime = dt;
    FeatureECS::ExecuteScheduler(world, IntegrateVelocitiesScheduler);

    // Iteration loop
    {
        for (uint32 iter = 0; iter < NumIterations; ++iter)
        {
            PHX_PROFILE_ZONE_SCOPED_N("IterationLoop");
            PHX_PROFILE_ZONE_VALUE(iter);
    
            // Determine contacts
            CalculateContactPairsJob->DeltaTime = dt;
            FeatureECS::ExecuteScheduler(world, CalculateContactPairsScheduler);
    
            WorldTaskQueue::Schedule(world, &PhysicsSystemDetail::ResolveContactPairsTask);
    
            // CalculateContactsTask depends on scratchBlock.Contacts.Num() from ResolveContactPairs
            WorldTaskQueue::Flush(world);
    
            WorldTaskQueue::ScheduleParallelRange(world, scratchBlock.Contacts.GetNum(), 128, &PhysicsSystemDetail::CalculateContactsTask, dt);

            // Multi-pass solver
            for (uint32 i = 0; i < NumSolverSteps; ++i)
            {
                PHX_PROFILE_ZONE_SCOPED_N("OverlapSeparationLoop");
                PHX_PROFILE_ZONE_VALUE(i);

                WorldTaskQueue::ScheduleParallelRange(world, scratchBlock.Contacts.GetNum(), 128, &PhysicsSystemDetail::PGSTask, i);
            }

            // Run CalculateContacts + all PGS steps now so workers stay busy.
            // Integrate depends on the corrected velocities produced by PGS.
            WorldTaskQueue::Flush(world);

            // Integrate positions
            IntegrateJob->DeltaTime = dt;
            IntegrateJob->bAllowSleep = AllowSleep;
            FeatureECS::ExecuteScheduler(world, IntegrateScheduler);

            // Multi-pass overlap separation
            for (uint32 i = 0; i < NumSeparationSteps; ++i)
            {
                PHX_PROFILE_ZONE_SCOPED_N("OverlapSeparationLoop");
                PHX_PROFILE_ZONE_VALUE(i);

                WorldTaskQueue::ScheduleParallelRange(world, scratchBlock.SortedEntities.GetNum(), 128, &PhysicsSystemDetail::OverlapSeparationTask);
                WorldTaskQueue::ScheduleParallelRange(world, scratchBlock.Contacts.GetNum(), 128, &PhysicsSystemDetail::OverlapSeparationTask2, PenetrationThreshold, PenetrationCorrection);
            }

            // Run overlap separation now rather than leaving it for the external flush.
            WorldTaskQueue::Flush(world);
        }
    }
}

void PhysicsSystem::OnDebugRender(WorldConstRef world, const IDebugState& state, IDebugRenderer& renderer)
{
    ISystem::OnDebugRender(world, state, renderer);

    const FeaturePhysicsScratchBlock& scratchBlock = world.GetBlockRef<FeaturePhysicsScratchBlock>();

    for (const CollisionLine& collisionLine : scratchBlock.CollisionLines)
    {
        renderer.DrawLine(collisionLine.Line.Start, collisionLine.Line.End, Color(0, 255, 0));
    }

    if (DebugDrawContacts)
    {
        for (const Contact& contact : scratchBlock.Contacts)
        {
            const ContactPair& contactPair = scratchBlock.ContactPairs[contact.ContactPair];
            Vec2 v = contact.Normal * contact.Bias;
            Vec2 s = contactPair.TransformA->Transform.Position;
            Vec2 e = s + v;
            renderer.DrawLine(s, e, Color(255, 255, 255));
        }
    }
}
