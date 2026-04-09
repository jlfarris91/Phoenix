
#include "PhoenixPhysics/PhysicsSystem.h"
#include "PhoenixSim/Reflection/Registration.h"

#include "PhoenixSim/Color.h"
#include "PhoenixSim/Debug/Debug.h"
#include "PhoenixSim/Flags.h"
#include "PhoenixSim/MortonCode.h"
#include "PhoenixSim/Profiling.h"
#include "PhoenixSim/WorldTaskQueue.h"
#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/ECS/SystemJob.h"
#include "PhoenixPhysics/BodyComponent.h"
#include "PhoenixPhysics/FeaturePhysics.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::Physics;

constexpr uint8 SLEEP_TIMER = 1;

namespace Phoenix::Physics::PhysicsSystemDetail
{
    struct PopulateSortedEntitiesJob : IJob<TransformComponent&, BodyComponent&>
    {
        FeaturePhysicsScratchBlock* ScratchBlock = nullptr;

        void Execute(WorldConstRef /*world*/, EntityId entityId, CommandBuffer& /*cb*/,
                     TransformComponent& transformComp, BodyComponent& bodyComp) override
        {
            uint32 sortedEntityIndex = ScratchBlock->SortedEntityCount.fetch_add(1);
            ScratchBlock->SortedEntities[sortedEntityIndex] =
                EntityBody{entityId, &transformComp, &bodyComp, transformComp.ZCode};
        }
    };

    struct SortEntitiesByZCodeTask : ITask
    {
        FeaturePhysicsScratchBlock* ScratchBlock = nullptr;

        FName GetName() const override { return "SortEntitiesByZCode"_n; }

        void Run(WorldConstRef /*world*/, CommandBuffer& /*cb*/) override
        {
            PHX_PROFILE_ZONE_SCOPED;

            ScratchBlock->SortedEntities.SetSize(ScratchBlock->SortedEntityCount);

            std::sort(
                ScratchBlock->SortedEntities.begin(),
                ScratchBlock->SortedEntities.end(),
                [](const EntityBody& a, const EntityBody& b)
                {
                    return a.ZCode < b.ZCode;
                });
        }
    };

    struct IntegrateVelocitiesJob : IJob<BodyComponent&>
    {
        DeltaTime DeltaTime;

        void Execute(WorldConstRef /*world*/, EntityId /*entityId*/, CommandBuffer& /*cb*/,
                     BodyComponent& bodyComp) override
        {
            PHX_PROFILE_ZONE_SCOPED_N("IntegrateVelocitiesJob");

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

        void Execute(WorldConstRef /*world*/, EntityId entityIdA, CommandBuffer& /*cb*/,
                     TransformComponent& transformCompA, BodyComponent& bodyCompA) override
        {
            PHX_PROFILE_ZONE_SCOPED_N("CalculateContactPairsJob");

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
                            return false;
                        if ((bodyCompA.CollisionMask & eb.BodyComponent->CollisionMask) == 0)
                            return false;
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
                    continue;

                entityid_t loId = Min(entityIdA, entityBodyB->EntityId);
                entityid_t hiId = Max(entityIdA, entityBodyB->EntityId);
                uint64 key = hiId; key = key << 32 | loId;

                uint32 contactIndex = ScratchBlock->ContactPairsCount.fetch_add(1);
                if (contactIndex >= ScratchBlock->ContactPairs.GetCapacity())
                    break;

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

        FeaturePhysicsScratchBlock& scratchBlock = world.GetBlockRef<FeaturePhysicsScratchBlock>();

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

        FeaturePhysicsScratchBlock& scratchBlock = world.GetBlockRef<FeaturePhysicsScratchBlock>();

        scratchBlock.ContactPairs.SetSize(scratchBlock.ContactPairsCount);

        {
            PHX_PROFILE_ZONE_SCOPED_N("SortContactPairs");

            std::sort(
                scratchBlock.ContactPairs.begin(),
                scratchBlock.ContactPairs.end(),
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

        FeaturePhysicsScratchBlock& scratchBlock = world.GetBlockRef<FeaturePhysicsScratchBlock>();

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

        void Execute(WorldConstRef /*world*/, EntityId /*entityId*/, CommandBuffer& /*cb*/,
                     TransformComponent& transformComp, BodyComponent& bodyComp) override
        {
            PHX_PROFILE_ZONE_SCOPED_N("IntegrateJob");

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

        FeaturePhysicsScratchBlock& scratchBlock = world.GetBlockRef<FeaturePhysicsScratchBlock>();

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

    void OverlapSeparationTask2(WorldRef world, uint32 startIndex, uint32 count, Value penetrationThreshold, Value penetrationCorrection)
    {
        PHX_PROFILE_ZONE_SCOPED;

        FeaturePhysicsScratchBlock& scratchBlock = world.GetBlockRef<FeaturePhysicsScratchBlock>();

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

void PhysicsSystem::OnWorldInitialize(WorldRef world)
{
    FeaturePhysicsScratchBlock& scratchBlock = world.GetBlockRef<FeaturePhysicsScratchBlock>();

    // --- Pre-update scheduler: populate sorted entities, then sort ---
    {
        auto populate = std::make_unique<PhysicsSystemDetail::PopulateSortedEntitiesJob>();
        populate->ScratchBlock = &scratchBlock;

        auto sort = std::make_unique<PhysicsSystemDetail::SortEntitiesByZCodeTask>();
        sort->ScratchBlock = &scratchBlock;

        JobHandle hPopulate = PreUpdateScheduler.RegisterJob(std::move(populate));
        JobHandle hSort     = PreUpdateScheduler.RegisterJob(std::move(sort));
        PreUpdateScheduler.AddDependency(hSort, hPopulate);

        const FeatureECSDynamicBlock& ecsBlock = world.GetBlockRef<FeatureECSDynamicBlock>();
        PreUpdateScheduler.Build(ecsBlock.ArchetypeManager);
    }

    // --- IntegrateVelocities scheduler ---
    {
        auto job = std::make_unique<PhysicsSystemDetail::IntegrateVelocitiesJob>();
        IntegrateVelocitiesJobPtr = job.get();
        IntegrateVelocitiesScheduler.RegisterJob(std::move(job));

        const FeatureECSDynamicBlock& ecsBlock = world.GetBlockRef<FeatureECSDynamicBlock>();
        IntegrateVelocitiesScheduler.Build(ecsBlock.ArchetypeManager);
    }

    // --- CalculateContactPairs scheduler ---
    {
        auto job = std::make_unique<PhysicsSystemDetail::CalculateContactPairsJob>();
        job->ScratchBlock = &scratchBlock;
        CalculateContactPairsJobPtr = job.get();
        CalculateContactPairsScheduler.RegisterJob(std::move(job));

        const FeatureECSDynamicBlock& ecsBlock = world.GetBlockRef<FeatureECSDynamicBlock>();
        CalculateContactPairsScheduler.Build(ecsBlock.ArchetypeManager);
    }

    // --- Integrate scheduler ---
    {
        auto job = std::make_unique<PhysicsSystemDetail::IntegrateJob>();
        IntegrateJobPtr = job.get();
        IntegrateScheduler.RegisterJob(std::move(job));

        const FeatureECSDynamicBlock& ecsBlock = world.GetBlockRef<FeatureECSDynamicBlock>();
        IntegrateScheduler.Build(ecsBlock.ArchetypeManager);
    }
}

void PhysicsSystem::OnPreWorldUpdate(WorldRef world, const SystemUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    FeaturePhysicsScratchBlock& scratchBlock = world.GetBlockRef<FeaturePhysicsScratchBlock>();

    scratchBlock.SortedEntities.Reset();
    scratchBlock.SortedEntityCount = 0;
    scratchBlock.ContactPairs.Reset();
    scratchBlock.ContactPairsCount = 0;

    FeatureECS::ExecuteScheduler(world, PreUpdateScheduler);
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
    IntegrateVelocitiesJobPtr->DeltaTime = dt;
    FeatureECS::ExecuteScheduler(world, IntegrateVelocitiesScheduler);

    for (uint32 iter = 0; iter < NumIterations; ++iter)
    {
        // Determine contacts
        CalculateContactPairsJobPtr->DeltaTime = dt;
        FeatureECS::ExecuteScheduler(world, CalculateContactPairsScheduler);

        WorldTaskQueue::Schedule(world, &PhysicsSystemDetail::ResolveContactPairsTask);

        // CalculateContactsTask depends on scratchBlock.Contacts.Num() from ResolveContactPairs
        WorldTaskQueue::Flush(world);

        WorldTaskQueue::ScheduleParallelRange(world, scratchBlock.Contacts.GetNum(), 128, &PhysicsSystemDetail::CalculateContactsTask, dt);

        // Multi-pass solver
        for (uint32 i = 0; i < NumSolverSteps; ++i)
            WorldTaskQueue::ScheduleParallelRange(world, scratchBlock.Contacts.GetNum(), 128, &PhysicsSystemDetail::PGSTask, i);

        // Integrate positions
        IntegrateJobPtr->DeltaTime = dt;
        IntegrateJobPtr->bAllowSleep = AllowSleep;
        FeatureECS::ExecuteScheduler(world, IntegrateScheduler);

        // Multi-pass overlap separation
        for (uint32 i = 0; i < NumSeparationSteps; ++i)
        {
            WorldTaskQueue::ScheduleParallelRange(world, scratchBlock.SortedEntities.GetNum(), 128, &PhysicsSystemDetail::OverlapSeparationTask);
            WorldTaskQueue::ScheduleParallelRange(world, scratchBlock.Contacts.GetNum(), 128, &PhysicsSystemDetail::OverlapSeparationTask2, PenetrationThreshold, PenetrationCorrection);
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
