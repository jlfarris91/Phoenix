
#include "Phoenix.Sim.ECS/FeatureECS.h"
#include "Phoenix/Reflection/Registration.h"

#include "Phoenix.Sim/MortonCode.h"
#include "Phoenix.Sim/Profiling.h"
#include "Phoenix.Sim/Session.h"
#include "Phoenix.Sim/WorldTaskQueue.h"
#include "Phoenix.Sim.ECS/ECSCommands.h"
#include "Phoenix.Sim.ECS/JobScheduler.h"
#include "Phoenix.Sim.ECS/System.h"
#include "Phoenix.Sim.ECS/SystemJob.h"
#include "Phoenix.Sim.Blackboard/FeatureBlackboard.h"
#include "Phoenix.Sim/Tasks/FeatureTask.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::Blackboard;

namespace FeatureECSDetail
{
    // Phase 1: assign non-overlapping write ranges to each batch and pre-size the output array.
    // Runs once per frame on the world thread before any parallel populate work.
    struct PrePartitionSortedEntitiesTask : ITask
    {
        JobNode* PopulateNode = nullptr;
        FeatureECSScratchBlock* ScratchBlock = nullptr;

        const char* GetName() const override { return "ECS.PrePartitionSortedEntitiesTask"; }

        void Run(WorldConstRef /*world*/, CommandBuffer& /*cb*/) override
        {
            ScratchBlock->PrevSortedEntityCount = ScratchBlock->SortedEntities.GetNum();

            uint32 total = 0;
            for (JobBatch& batch : PopulateNode->Batches)
            {
                batch.UserData = total;
                total += batch.List ? batch.List->GetNumInstances() : 0;
            }

            // Invalidate scatter slots up to the high-water mark from the previous frame.
            std::memset(ScratchBlock->SortedEntities.GetData(), 0xFF,
                        ScratchBlock->SortedEntities.GetNum() * sizeof(EntityTransform));

            ScratchBlock->SortedEntities.SetNum(total);
            ScratchBlock->SortedEntityCount = 0;
        }
    };

    // Phase 2: compute Z-codes and write entities into pre-assigned batch ranges.
    // Batches run in parallel; each thread writes to a non-overlapping slice — no contention.
    // Skipped (invalid) entities reduce the actual written count tracked via SortedEntityCount.
    class PopulateSortedEntitiesJob : public IJob<TransformComponent&>
    {
    public:
        FeatureECSDynamicBlock* DynamicBlock = nullptr;
        FeatureECSScratchBlock* ScratchBlock = nullptr;

        const char* GetName() const override { return "ECS.PopulateSortedEntitiesJob"; }

        void BeginBatch(WorldConstRef /*world*/, const JobBatch& batch, CommandBuffer& /*cb*/) override
        {
            tl_BatchStart = batch.UserData;
            tl_WriteCount = 0;
        }

        void Execute(WorldConstRef /*world*/, EntityId id, CommandBuffer& /*cb*/, TransformComponent& t) override
        {
            t.ZCode = ToMortonCode(t.Transform.Position);
            auto tl_writeIdx = tl_BatchStart + tl_WriteCount;
            ScratchBlock->SortedEntities[tl_writeIdx] = EntityTransform { id, &t, t.ZCode };
            ++tl_WriteCount;
        }

        void EndBatch(WorldConstRef /*world*/, const JobBatch& /*batch*/, CommandBuffer& /*cb*/) override
        {
            // One atomic add per batch (not per entity) to accumulate the actual valid entity count.
            ScratchBlock->SortedEntityCount.fetch_add(tl_WriteCount, std::memory_order_relaxed);
        }

    private:
        // thread_local: each worker thread runs one batch at a time, so these are
        // independent per-thread — no contention between parallel batches.
        static thread_local uint32 tl_BatchStart;
        static thread_local uint32 tl_WriteCount;
    };

    thread_local uint32 PopulateSortedEntitiesJob::tl_BatchStart = 0;
    thread_local uint32 PopulateSortedEntitiesJob::tl_WriteCount = 0;

    // Phase 3: trim the output array to the actual valid entity count and sort by Z-code.
    struct SortEntitiesTask : ITask
    {
        FeatureECSDynamicBlock* DynamicBlock = nullptr;
        FeatureECSScratchBlock* ScratchBlock = nullptr;

        const char* GetName() const override { return "ECS.SortEntitiesTask"; }

        void Run(WorldConstRef /*world*/, CommandBuffer& /*cb*/) override
        {
#if 0
            // Test to ensure that duplicate entities are not added to SortedEntities
            for (uint32 i = 0; i < ScratchBlock->SortedEntityCount; ++i)
            {
                for (uint32 j = i + 1; j < ScratchBlock->SortedEntityCount; ++j)
                {
                    if (ScratchBlock->SortedEntities[i].EntityId != EntityId::Invalid &&
                        ScratchBlock->SortedEntities[i].EntityId == ScratchBlock->SortedEntities[j].EntityId)
                    {
                        __debugbreak();
                    }
                }
            }
#endif

            std::ranges::sort(ScratchBlock->SortedEntities, [](const EntityTransform& a, const EntityTransform& b)
            {
                if (a.EntityId == EntityId::Invalid)
                {
                    return false;
                }
                if (b.EntityId == EntityId::Invalid)
                {
                    return true;
                }
                return a.ZCode < b.ZCode;
            });

            ScratchBlock->SortedEntities.SetSize(ScratchBlock->SortedEntityCount);

            std::memset(ScratchBlock->SortedEntityIndex.GetData(), 0xFF,
                        DynamicBlock->Entities.GetNumHighWaterMark() * sizeof(uint32));
            ScratchBlock->SortedEntityIndex.SetSize(DynamicBlock->Entities.GetNumHighWaterMark());

            // Build entityId → sorted-position lookup for downstream scatter consumers.
            for (uint32 i = 0; i < ScratchBlock->SortedEntities.GetNum(); ++i)
            {
                auto entityIdx = DynamicBlock->Entities.GetEntityIndex(ScratchBlock->SortedEntities[i].EntityId) - 1;
#if 0
                // Test to ensure that no two entities are occupying the same slot in SortedEntityIndex
                if (ScratchBlock->SortedEntityIndex[entityIdx] != Index<uint32>::None)
                {
                    auto existingIdx = ScratchBlock->SortedEntityIndex[entityIdx];
                    EntityTransform& entityTransform = ScratchBlock->SortedEntities[existingIdx];
                    auto existingEntity = DynamicBlock->Entities.GetEntityPtr(entityTransform.EntityId);
                    auto existingEntityIdx = DynamicBlock->Entities.GetEntityIndex(entityTransform.EntityId);
                    __debugbreak();
                }
#endif
                PHX_ASSERT(ScratchBlock->SortedEntityIndex[entityIdx] == Index<uint32>::None);
                ScratchBlock->SortedEntityIndex[entityIdx] = i;
            }
        }
    };
}

// ============================================================================
// FeatureECSDynamicBlock
// ============================================================================

void FeatureECSDynamicBlock::Construct(BlockBufferAllocator& allocator, const Config& config)
{
    Entities.Construct(allocator, config.MaxEntities);
    ArchetypeManager.Construct(allocator, config.ArchetypeManager);
    Tags.Construct(allocator, config.MaxTags);
    Groups.Construct(allocator, config.MaxGroups);
}

BlockBufferLayout FeatureECSDynamicBlock::StaticLayout(const Config& config)
{
    return BlockBufferLayout::For<FeatureECSDynamicBlock>()
        .Container<FixedEntityList>("Entities", config.MaxEntities)
        .Container<ECS::ArchetypeManager>("ArchetypeManager", config.ArchetypeManager)
        .Container<FixedTagList>("Tags", config.MaxTags)
        .Container<FixedGroupList>("Groups", config.MaxGroups);
}

void FeatureECSScratchBlock::Construct(BlockBufferAllocator& allocator, const Config& config)
{
    SortedEntities.Construct(allocator, config.MaxEntities);
    SortedEntityIndex.Construct(allocator, config.MaxEntities);
}

BlockBufferLayout FeatureECSScratchBlock::StaticLayout(const Config& config)
{
    return BlockBufferLayout::For<FeatureECSScratchBlock>()
        .Container<TFixedArray<EntityTransform>>("SortedEntities", config.MaxEntities)
        .Container<TFixedArray<uint32>>("SortedEntityIndex", config.MaxEntities);
}

FOnEntityAcquired& FeatureECS::OnEntityAcquired()
{
    return EntityAcquiredEvent;
}

FOnEntityReleasing& FeatureECS::OnEntityReleasing()
{
    return EntityReleasedEvent;
}

FOnEntityReleased& FeatureECS::OnEntityReleased()
{
    return EntityDestroyedEvent;
}

void FeatureECS::RegisterSystem(const std::shared_ptr<ISystem>& system)
{
    Systems.push_back(system);
}

bool FeatureECS::UnregisterSystem(const std::shared_ptr<ISystem>& system)
{
    auto iter = std::ranges::find(Systems, system);
    if (iter == Systems.end())
        return false;
    Systems.erase(iter);
    return true;
}

const std::vector<std::shared_ptr<ISystem>>& FeatureECS::GetSystems() const
{
    return Systems;
}

const FixedEntityList* FeatureECS::GetEntities(WorldConstRef world)
{
    const FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    return block ? &block->Entities : nullptr;
}

bool FeatureECS::IsEntityValid(WorldConstRef world, EntityId entityId)
{
    return GetEntityPtr(world, entityId) != nullptr;
}

Entity* FeatureECS::GetEntityPtr(WorldRef world, EntityId entityId)
{
    PHX_PROFILE_ZONE_SCOPED;

    FeatureECSDynamicBlock& block = world.GetBlockRef<FeatureECSDynamicBlock>();
    return block.Entities.GetEntityPtr(entityId);
}

const Entity* FeatureECS::GetEntityPtr(WorldConstRef world, EntityId entityId)
{
    PHX_PROFILE_ZONE_SCOPED;

    const FeatureECSDynamicBlock& block = world.GetBlockRef<FeatureECSDynamicBlock>();
    return block.Entities.GetEntityPtr(entityId);
}

Entity& FeatureECS::GetEntityRef(WorldRef world, EntityId entityId)
{
    PHX_PROFILE_ZONE_SCOPED;

    FeatureECSDynamicBlock& block = world.GetBlockRef<FeatureECSDynamicBlock>();
    return block.Entities.GetEntityRef(entityId);
}

const Entity& FeatureECS::GetEntityRef(WorldConstRef world, EntityId entityId)
{
    PHX_PROFILE_ZONE_SCOPED;

    const FeatureECSDynamicBlock& block = world.GetBlockRef<FeatureECSDynamicBlock>();
    return block.Entities.GetEntityRef(entityId);
}

EntityId FeatureECS::StaticAcquireEntity(WorldRef world, const FName& kind)
{
    std::shared_ptr<FeatureECS> feature = GetFeature<FeatureECS>(world);
    return feature ? feature->AcquireEntity(world, kind) : EntityId::Invalid;
}

EntityId FeatureECS::AcquireEntity(WorldRef world, const FName& kind) const
{
    PHX_PROFILE_ZONE_SCOPED;

    FeatureECSDynamicBlock& block = world.GetBlockRef<FeatureECSDynamicBlock>();

    EntityId entityId = block.Entities.Acquire(kind);
    if (entityId == EntityId::Invalid)
    {
        return EntityId::Invalid;
    }

    // Automatically acquire an archetype if the kind matches one
    if (block.ArchetypeManager.IsArchetypeRegistered(kind))
    {
        block.ArchetypeManager.Acquire(entityId, kind);
    }

    EntityAcquiredEvent.Broadcast(world, entityId);

    return entityId;
}

bool FeatureECS::StaticReleaseEntity(WorldRef world, EntityId entityId)
{
    std::shared_ptr<FeatureECS> feature = GetFeature<FeatureECS>(world);
    return feature && feature->ReleaseEntity(world, entityId);
}

bool FeatureECS::ReleaseEntity(WorldRef world, EntityId entityId) const
{
    FeatureECSDynamicBlock& block = world.GetBlockRef<FeatureECSDynamicBlock>();

    if (!block.Entities.IsValid(entityId))
    {
        return false;
    }

    // Broadcast before releasing so that handlers can still access entity data if needed.
    EntityReleasedEvent.Broadcast(world, entityId);

    block.Entities.Release(entityId);

    return true;
}

bool FeatureECS::SetEntityKind(WorldRef world, EntityId entityId, const FName& kind)
{
    FeatureECSDynamicBlock& block = world.GetBlockRef<FeatureECSDynamicBlock>();

    Entity* entity = block.Entities.GetEntityPtr(entityId);
    if (!entity)
    {
        return false;
    }

    if (entity->Kind == kind)
    {
        return true;
    }

    // If the entity has an archetype then release it now
    (void)block.ArchetypeManager.Release(entity->Id);

    entity->Kind = kind;

    // Automatically acquire an archetype if the kind matches one
    if (block.ArchetypeManager.IsArchetypeRegistered(kind))
    {
        block.ArchetypeManager.Acquire(entity->Id, kind);
    }

    return true;
}

bool FeatureECS::RegisterArchetypeDefinition(WorldRef world, const ArchetypeDefinition& definition)
{
    FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    if (!block)
    {
        return false;
    }

    return block->ArchetypeManager.RegisterArchetypeDefinition(definition);
}

bool FeatureECS::UnregisterArchetypeDefinition(WorldRef world, const ArchetypeDefinition& definition)
{
    FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    if (!block)
    {
        return false;
    }

    return block->ArchetypeManager.UnregisterArchetypeDefinition(definition);
}

bool FeatureECS::HasArchetypeDefinition(WorldConstRef world, const FName& name)
{
    const FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    if (!block)
    {
        return false;
    }

    return block->ArchetypeManager.IsArchetypeRegistered(name);
}

bool FeatureECS::RegisterComponentDefinition(WorldRef world, const ComponentDefinition& definition)
{
    FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    if (!block)
    {
        return false;
    }

    return block->ArchetypeManager.RegisterComponentDefinition(definition);
}

bool FeatureECS::UnregisterComponentDefinition(WorldRef world, const FName& componentType)
{
    FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    if (!block)
    {
        return false;
    }

    return block->ArchetypeManager.UnregisterComponentDefinition(componentType);
}

bool FeatureECS::HasComponentDefinition(WorldRef world, const FName& componentType)
{
    const FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    if (!block)
    {
        return false;
    }

    return block->ArchetypeManager.IsComponentRegistered(componentType);
}

IComponent* FeatureECS::GetComponent(
    WorldRef world,
    EntityId entityId,
    const FName& componentType)
{
    PHX_PROFILE_ZONE_SCOPED;

    FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    if (!block)
    {
        return nullptr;
    }

    if (!block->Entities.IsValid(entityId))
    {
        return nullptr;
    }

    return static_cast<IComponent*>(block->ArchetypeManager.GetComponent(entityId, componentType));
}

const IComponent* FeatureECS::GetComponent(
    WorldConstRef world,
    EntityId entityId,
    const FName& componentType)
{
    PHX_PROFILE_ZONE_SCOPED;

    const FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    if (!block)
    {
        return nullptr;
    }

    if (!block->Entities.IsValid(entityId))
    {
        return nullptr;
    }

    return static_cast<const IComponent*>(block->ArchetypeManager.GetComponent(entityId, componentType));
}

IComponent& FeatureECS::GetComponentRef(
    WorldRef world,
    EntityId entityId,
    const FName& componentType)
{
    IComponent* component = GetComponent(world, entityId, componentType);
    PHX_ASSERT(component);
    return *component;
}

const IComponent& FeatureECS::GetComponentRef(
    WorldConstRef world,
    EntityId entityId,
    const FName& componentType)
{
    const IComponent* component = GetComponent(world, entityId, componentType);
    PHX_ASSERT(component);
    return *component;
}

bool FeatureECS::HasComponent(WorldConstRef world, EntityId entityId, const FName& componentType)
{
    const FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    return block && block->ArchetypeManager.GetComponent(entityId, componentType) != nullptr;
}

IComponent* FeatureECS::AddComponent(
    WorldRef world,
    EntityId entityId,
    const FName& componentType,
    const void* componentData)
{
    FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    if (!block)
    {
        return nullptr;
    }

    Entity* entity = GetEntityPtr(world, entityId);
    if (!entity)
    {
        return nullptr;
    }

    return static_cast<IComponent*>(block->ArchetypeManager.AddComponent(entity->Id, componentType, componentData));
}

bool FeatureECS::RemoveComponent(WorldRef world, EntityId entityId, const FName& componentType)
{
    FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    if (!block)
    {
        return false;
    }

    Entity* entity = GetEntityPtr(world, entityId);
    if (!entity)
    {
        return false;
    }

    return block->ArchetypeManager.RemoveComponent(entity->Id, componentType);
}

uint32 FeatureECS::RemoveAllComponents(WorldRef world, EntityId entityId)
{
    FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    if (!block)
    {
        return false;
    }

    Entity* entity = GetEntityPtr(world, entityId);
    if (!entity)
    {
        return false;
    }

    return block->ArchetypeManager.RemoveAllComponents(entity->Id);
}

const decltype(FeatureECSDynamicBlock::Tags)* FeatureECS::GetTags(WorldConstRef world)
{
    const FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    return block ? &block->Tags : nullptr;
}

bool FeatureECS::HasTag(WorldConstRef world, EntityId entityId, const FName& tag)
{
    const FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    return block && block->Tags.HasTag(entityId, tag);
}

bool FeatureECS::AddTag(WorldRef world, EntityId entityId, const FName& tag)
{
    FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    return block && block->Tags.AddTag(entityId, tag);
}

bool FeatureECS::RemoveTag(WorldRef world, EntityId entityId, const FName& tag)
{
    FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    return block && block->Tags.RemoveTag(entityId, tag);
}

uint32 FeatureECS::RemoveAllTags(WorldRef world, EntityId entityId)
{
    FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    return block ? block->Tags.RemoveAllTags(entityId) : 0;
}

const decltype(FeatureECSDynamicBlock::Groups)* FeatureECS::GetGroups(WorldConstRef world)
{
    const FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    return block ? &block->Groups : nullptr;
}

bool FeatureECS::GroupContainsEntity(WorldConstRef world, EntityId group, EntityId entity)
{
    const FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    return block && block->Groups.ContainsEntity(group, entity);
}

bool FeatureECS::AddEntityToGroup(WorldRef world, EntityId group, EntityId entity)
{
    FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    return block && block->Groups.AddEntity(group, entity);
}

bool FeatureECS::RemoveEntityFromGroup(WorldRef world, EntityId group, EntityId entity)
{
    FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    return block && block->Groups.RemoveEntity(group, entity);
}

uint32 FeatureECS::RemoveEntityFromAllGroups(WorldRef world, EntityId entity)
{
    FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    return block && block->Groups.RemoveEntityFromAllGroups(entity);
}

uint32 FeatureECS::ClearGroup(WorldRef world, EntityId group)
{
    FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    return block ? block->Groups.ClearGroup(group) : 0;
}

uint32 FeatureECS::GetGroupSize(WorldConstRef world, EntityId group)
{
    const FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    return block ? block->Groups.GetNumEntities(group) : 0;
}

EntityId FeatureECS::GetFirstEntityInGroup(WorldConstRef world, EntityId group)
{
    uint32 index;
    return GetFirstEntityInGroup(world, group, index);
}

EntityId FeatureECS::GetFirstEntityInGroup(WorldConstRef world, EntityId group, uint32& outIndex)
{
    const FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    return block ? block->Groups.GetFirstEntity(group, outIndex) : EntityId::Invalid;
}

EntityId FeatureECS::GetNextEntityInGroup(WorldConstRef world, EntityId group, uint32 currIndex, uint32& outIndex)
{
    const FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
    return block ? block->Groups.GetNextEntity(group, currIndex, outIndex) : EntityId::Invalid;
}

blackboard_key_t FeatureECS::CreateBlackboardKey(const EntityId& id, const FName& key, blackboard_type_t type)
{
    return BlackboardKey::Create(key, id, type);
}

bool FeatureECS::HasBlackboardValue(
    WorldConstRef world,
    const EntityId& id,
    const FName& key,
    blackboard_type_t type)
{
    const FixedBlackboard& blackboard = FeatureBlackboard::GetBlackboard(world);
    blackboard_key_t fullKey = CreateBlackboardKey(id, key);
    return blackboard.HasValue(BlackboardQuery(fullKey, type));
}

bool FeatureECS::SetBlackboardValueRaw(
    WorldRef world,
    const EntityId& id,
    const FName& key,
    blackboard_value_t value,
    blackboard_type_t type)
{
    FixedBlackboard& blackboard = FeatureBlackboard::GetBlackboard(world);
    blackboard_key_t fullKey = CreateBlackboardKey(id, key, type);
    return blackboard.SetValue(fullKey, value);
}

bool FeatureECS::TryGetBlackboardValueRaw(
    WorldConstRef world,
    const EntityId& id,
    const FName& key,
    blackboard_value_t& outValue,
    blackboard_type_t expectedType)
{
    const FixedBlackboard& blackboard = FeatureBlackboard::GetBlackboard(world);
    blackboard_key_t fullKey = CreateBlackboardKey(id, key);
    return blackboard.GetValue(BlackboardQuery(fullKey, expectedType), outValue);
}

blackboard_value_t FeatureECS::GetBlackboardValueRaw(
    WorldConstRef world,
    const EntityId& id,
    const FName& key,
    blackboard_type_t expectedType)
{
    const FixedBlackboard& blackboard = FeatureBlackboard::GetBlackboard(world);
    blackboard_key_t fullKey = CreateBlackboardKey(id, key);
    blackboard_value_t value;
    blackboard.GetValue(BlackboardQuery(fullKey, expectedType), value);
    return value;
}

CommandBuffer& FeatureECS::GetCommandBuffer(WorldConstRef world)
{
    std::shared_ptr<FeatureECS> feature = GetFeature<FeatureECS>(world);
    return *feature->CommandBuffers[GetCurrentThreadIndex()];
}

void FeatureECS::RegisterCommandHandler(WorldRef world, FName commandId, TCommandHandler handler)
{
    std::shared_ptr<FeatureECS> feature = GetFeature<FeatureECS>(world);
    feature->CommandHandlers[commandId] = std::move(handler);
}

JobHandle FeatureECS::RegisterJob(WorldRef world, std::unique_ptr<IJobBase> job, EJobPhase phase)
{
    std::shared_ptr<FeatureECS> feature = GetFeature<FeatureECS>(world);
    IJobBase* raw = job.get();
    feature->OwnedJobs.push_back(std::move(job));
    return feature->GetMutableScheduler(world, phase).RegisterJob(raw);
}

JobHandle FeatureECS::RegisterJob(WorldRef world, IJobBase* job, EJobPhase phase)
{
    std::shared_ptr<FeatureECS> feature = GetFeature<FeatureECS>(world);
    return feature->GetMutableScheduler(world, phase).RegisterJob(job);
}

void FeatureECS::AddJobDependency(WorldRef world, EJobPhase phase, JobHandle after, JobHandle before)
{
    std::shared_ptr<FeatureECS> feature = GetFeature<FeatureECS>(world);
    feature->GetMutableScheduler(world, phase).AddDependency(after, before);
}

JobHandle FeatureECS::GetPreUpdateSortJobHandle(WorldConstRef world)
{
    if (std::shared_ptr<FeatureECS> feature = GetFeature<FeatureECS>(world))
    {
        ScopedWorldData& worldData = feature->GetScopedWorldData(world);
        return worldData.PreUpdateSortJobHandle;
    }
    return InvalidJobHandle;
}

void FeatureECS::ExecuteScheduler(WorldRef world, JobScheduler& scheduler)
{
    std::shared_ptr<FeatureECS> feature = GetFeature<FeatureECS>(world);
    if (!feature)
        return;

    const FeatureECSDynamicBlock& dynamicBlock = world.GetBlockRef<FeatureECSDynamicBlock>();
    scheduler.RebuildBatchesIfDirty(dynamicBlock.ArchetypeManager);

    std::vector<CommandBuffer*> threadCbs;
    threadCbs.reserve(feature->CommandBuffers.size());
    for (const std::unique_ptr<CommandBuffer>& cb : feature->CommandBuffers)
        threadCbs.push_back(cb.get());

    if (feature->bAllowParallelJobs)
    {
        std::shared_ptr<TaskQueue> taskQueue = TaskQueue::GetTaskQueue((uint32)world.GetId());
        scheduler.Execute(world, *taskQueue, threadCbs);
    }
    else
    {
        scheduler.ExecuteSerial(world, threadCbs);
    }
}

const JobScheduler& FeatureECS::GetScheduler(WorldConstRef world, EJobPhase phase)
{
    std::shared_ptr<FeatureECS> feature = GetFeature<FeatureECS>(world);
    PHX_ASSERT(feature);
    return feature->GetMutableScheduler(world, phase);
}

void FeatureECS::RegisterScheduler(WorldRef world, const JobScheduler& scheduler)
{
    std::shared_ptr<FeatureECS> feature = GetFeature<FeatureECS>(world);
    PHX_ASSERT(feature);
    ScopedWorldData& worldData = feature->GetScopedWorldData(world);
    worldData.NamedSchedulers.emplace_back(scheduler.GetName(), &scheduler);
}

std::vector<std::pair<FName, const JobScheduler*>> FeatureECS::GetNamedSchedulers(WorldConstRef world)
{
    std::shared_ptr<FeatureECS> feature = GetFeature<FeatureECS>(world);
    PHX_ASSERT(feature);
    ScopedWorldData& worldData = feature->GetScopedWorldData(world);
    return worldData.NamedSchedulers;
}

const Transform2D* FeatureECS::GetLocalTransformPtr(WorldConstRef world, EntityId entityId)
{
    PHX_PROFILE_ZONE_SCOPED;
    const TransformComponent* comp = GetComponent<TransformComponent>(world, entityId);
    return comp ? &comp->Transform : nullptr;
}

const Transform2D* FeatureECS::GetWorldTransformPtr(WorldConstRef world, EntityId entityId)
{
    PHX_PROFILE_ZONE_SCOPED;
    const TransformComponent* comp = GetComponent<TransformComponent>(world, entityId);
    return comp ? &comp->Transform : nullptr;
}

Vec2 FeatureECS::GetLocalPosition(WorldConstRef world, EntityId entityId)
{
    auto entityTransform = GetLocalTransformPtr(world, entityId);
    return entityTransform ? entityTransform->Position : Vec2::Zero;
}

Vec2 FeatureECS::GetWorldPosition(WorldConstRef world, EntityId entityId)
{
    auto entityTransform = GetWorldTransformPtr(world, entityId);
    return entityTransform ? entityTransform->Position : Vec2::Zero;
}

Angle FeatureECS::GetLocalFacing(WorldConstRef world, EntityId entityId)
{
    auto entityTransform = GetWorldTransformPtr(world, entityId);
    return entityTransform ? entityTransform->Rotation : 0;
}

Angle FeatureECS::GetWorldFacing(WorldConstRef world, EntityId entityId)
{
    auto entityTransform = GetWorldTransformPtr(world, entityId);
    return entityTransform ? entityTransform->Rotation : 0;
}

Value FeatureECS::GetLocalScale(WorldConstRef world, EntityId entityId)
{
    auto entityTransform = GetWorldTransformPtr(world, entityId);
    return entityTransform ? entityTransform->Scale : 1;
}

Value FeatureECS::GetWorldScale(WorldConstRef world, EntityId entityId)
{
    auto entityTransform = GetWorldTransformPtr(world, entityId);
    return entityTransform ? entityTransform->Scale : 1;
}

EntityId FeatureECS::GetParent(WorldConstRef world, EntityId entityId)
{
    const TransformComponent* comp = GetComponent<TransformComponent>(world, entityId);
    return comp ? comp->AttachParent : EntityId::Invalid;
}

bool FeatureECS::IsInRange(WorldConstRef world, EntityId entity, EntityId target, Distance range)
{
    auto targetTransform = GetWorldTransformPtr(world, target);
    return targetTransform && IsInRange(world, entity, targetTransform->Position, range);
}

bool FeatureECS::IsInRange(WorldConstRef world, EntityId entity, const Vec2& target, Distance range)
{
    auto entityTransform = GetWorldTransformPtr(world, entity);
    return entityTransform && Vec2::Distance(entityTransform->Position, target) <= range;
}

bool FeatureECS::IsFacing(WorldConstRef world, EntityId entity, EntityId target, Angle threshold)
{
    auto targetTransform = GetWorldTransformPtr(world, target);
    return targetTransform && IsFacing(world, entity, targetTransform->Position, threshold);
}

bool FeatureECS::IsFacing(WorldConstRef world, EntityId entity, const Vec2& target, Angle threshold)
{
    auto entityTransform = GetWorldTransformPtr(world, entity);
    if (!entityTransform)
    {
        return false;
    }

    Angle entityFacing = entityTransform->Rotation;
    Angle targetFacing = (target - entityTransform->Position).AsRadians();

    return Abs(AngleDelta(entityFacing, targetFacing)) <= Deg2Rad(threshold);
}

void FeatureECS::QueryEntitiesInRange(
    WorldConstRef world,
    const Vec2& pos,
    Distance range,
    std::vector<EntityTransform>& outEntities,
    const EntityRangeQueryArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    const FeatureECSDynamicBlock* dynamicBlock = world.GetBlock<FeatureECSDynamicBlock>();
    const FeatureECSScratchBlock* scratchBlock = world.GetBlock<FeatureECSScratchBlock>();
    if (!dynamicBlock || !scratchBlock)
    {
        return;
    }

    // Query for overlapping morton ranges
    TMortonCodeRangeArray ranges;
    MortonCodeAABB aabb = ToMortonCodeAABB(pos, range);
    MortonCodeQuery(aabb, ranges);

    std::vector<EntityTransform*> overlappingEntities;
    ForEachInMortonCodeRanges<EntityTransform, &EntityTransform::ZCode>(
        scratchBlock->SortedEntities,
        ranges,
        [&](const EntityTransform& entityTransform)
        {
            const Entity* entity = dynamicBlock->Entities.GetEntityPtr(entityTransform.EntityId);
            if (!entity)
            {
                return;
            }

            if (args.Kinds.IsSet() && !args.Kinds.Get().contains(entity->Kind))
            {
                return;
            }

            const Vec2& entityPos = entityTransform.TransformComponent->Transform.Position;
            if (Vec2::Distance(pos, entityPos) < range)
            {
                outEntities.push_back(entityTransform);
            }
        });
}

void FeatureECS::QueryEntitiesInRect(
    WorldConstRef world,
    const Vec2& min,
    const Vec2& max,
    std::vector<EntityTransform>& outEntities,
    const EntityRangeQueryArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    const FeatureECSDynamicBlock* dynamicBlock = world.GetBlock<FeatureECSDynamicBlock>();
    const FeatureECSScratchBlock* scratchBlock = world.GetBlock<FeatureECSScratchBlock>();
    if (!dynamicBlock || !scratchBlock)
    {
        return;
    }

    // Query for overlapping morton ranges
    TMortonCodeRangeArray ranges;
    MortonCodeAABB aabb = ToMortonCodeAABB(min, max);
    MortonCodeQuery(aabb, ranges);

    std::vector<EntityTransform*> overlappingEntities;
    ForEachInMortonCodeRanges<EntityTransform, &EntityTransform::ZCode>(
        scratchBlock->SortedEntities,
        ranges,
        [&](const EntityTransform& entityBody)
        {
            outEntities.push_back(entityBody);
        });
}

void FeatureECS::Initialize(const std::shared_ptr<Phoenix::Session>& session)
{
    IFeature::Initialize(session);

    const uint32 n = HasThreadPool() ? GetThreadPool()->GetNumWorkers() + 1 : 1;
    for (uint32 i = 0; i < n; ++i)
    {
        CommandBuffers.push_back(std::make_unique<CommandBuffer>());
    }
}

void FeatureECS::OnPreUpdate(const FeatureUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    SystemUpdateArgs systemUpdateArgs;
    systemUpdateArgs.SimTime = args.SimTime;
    systemUpdateArgs.DeltaTime = OneDivBy(Time(Time::D));

    for (const std::shared_ptr<ISystem>& system : Systems)
    {
        system->OnPreUpdate(systemUpdateArgs);
    }
}

void FeatureECS::OnUpdate(const FeatureUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    SystemUpdateArgs systemUpdateArgs;
    systemUpdateArgs.SimTime = args.SimTime;
    systemUpdateArgs.DeltaTime = OneDivBy(Time(Time::D));

    for (const std::shared_ptr<ISystem>& system : Systems)
    {
        system->OnUpdate(systemUpdateArgs);
    }
}

void FeatureECS::OnPostUpdate(const FeatureUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    SystemUpdateArgs systemUpdateArgs;
    systemUpdateArgs.SimTime = args.SimTime;
    systemUpdateArgs.DeltaTime = OneDivBy(Time(Time::D));

    for (const std::shared_ptr<ISystem>& system : Systems)
    {
        system->OnPostUpdate(systemUpdateArgs);
    }
}

bool FeatureECS::OnPreHandleAction(const FeatureActionArgs& action)
{
    PHX_PROFILE_ZONE_SCOPED;

    SystemActionArgs systemActionArgs;
    systemActionArgs.SimTime = action.SimTime;
    systemActionArgs.Action = action.Action;

    for (const std::shared_ptr<ISystem>& system : Systems)
    {
        if (system->OnPreHandleAction(systemActionArgs))
        {
            return true;
        }
    }
    
    return false;
}

bool FeatureECS::OnHandleAction(const FeatureActionArgs& action)
{
    PHX_PROFILE_ZONE_SCOPED;

    SystemActionArgs systemActionArgs;
    systemActionArgs.SimTime = action.SimTime;
    systemActionArgs.Action = action.Action;

    for (const std::shared_ptr<ISystem>& system : Systems)
    {
        if (system->OnHandleAction(systemActionArgs))
        {
            return true;
        }
    }
    
    return false;
}

bool FeatureECS::OnPostHandleAction(const FeatureActionArgs& action)
{
    PHX_PROFILE_ZONE_SCOPED;

    SystemActionArgs systemActionArgs;
    systemActionArgs.SimTime = action.SimTime;
    systemActionArgs.Action = action.Action;

    for (const std::shared_ptr<ISystem>& system : Systems)
    {
        if (system->OnPostHandleAction(systemActionArgs))
        {
            return true;
        }
    }
    
    return false;
}

void FeatureECS::OnWorldLayout(const WorldLayoutContext& context, BlockBufferConfigBuilder& builder)
{
    IFeature::OnWorldLayout(context, builder);

    FeatureECSDynamicBlock::Config dynamicBlockConfig;
    dynamicBlockConfig.MaxEntities = PHX_ECS_MAX_ENTITIES;
    dynamicBlockConfig.MaxTags = PHX_ECS_MAX_TAGS;
    dynamicBlockConfig.MaxGroups = PHX_ECS_MAX_GROUPS;
    dynamicBlockConfig.ArchetypeManager.MaxComponentDefs = PHX_ECS_MAX_COMPONENT_DEFS;
    dynamicBlockConfig.ArchetypeManager.MaxArchetypeDefs = PHX_ECS_MAX_ARCHETYPE_DEFS;
    dynamicBlockConfig.ArchetypeManager.MaxArchetypeLists = PHX_ECS_MAX_ARCHETYPE_LISTS;
    dynamicBlockConfig.ArchetypeManager.ArchetypeListSize = PHX_ECS_ARCHETYPE_LIST_SIZE;

    if (const FeatureJsonConfig* featureConfig = context.Config.GetFeatureConfig(GetFeatureId()))
    {
        const nlohmann::json& featureConfigData = featureConfig->GetData();

        dynamicBlockConfig.MaxEntities = featureConfigData.value("max_entities", dynamicBlockConfig.MaxEntities);
        dynamicBlockConfig.MaxTags = featureConfigData.value("max_tags", dynamicBlockConfig.MaxTags);
        dynamicBlockConfig.MaxGroups = featureConfigData.value("max_groups", dynamicBlockConfig.MaxGroups);
        dynamicBlockConfig.ArchetypeManager.MaxComponentDefs = featureConfigData.value("max_component_defs", dynamicBlockConfig.ArchetypeManager.MaxComponentDefs);
        dynamicBlockConfig.ArchetypeManager.MaxArchetypeDefs = featureConfigData.value("max_archetype_defs", dynamicBlockConfig.ArchetypeManager.MaxArchetypeDefs);
        dynamicBlockConfig.ArchetypeManager.MaxArchetypeLists = featureConfigData.value("max_archetype_lists", dynamicBlockConfig.ArchetypeManager.MaxArchetypeLists);
        dynamicBlockConfig.ArchetypeManager.ArchetypeListSize = featureConfigData.value("archetype_list_size", dynamicBlockConfig.ArchetypeManager.ArchetypeListSize);
    }

    dynamicBlockConfig.ArchetypeManager.MaxEntities = dynamicBlockConfig.MaxEntities;

    builder.RegisterBlockWithAlloc<FeatureECSDynamicBlock>(EBufferBlockType::Dynamic, dynamicBlockConfig);

    FeatureECSScratchBlock::Config scratchBlockConfig;
    scratchBlockConfig.MaxEntities = dynamicBlockConfig.MaxEntities;

    builder.RegisterBlockWithAlloc<FeatureECSScratchBlock>(EBufferBlockType::Scratch, scratchBlockConfig);
}

void FeatureECS::OnWorldInitialize(WorldRef world)
{
    PHX_PROFILE_ZONE_SCOPED;

    TaskQueue::CreateTaskQueue((uint32)world.GetId());

    RegisterECSCommandHandlers();

    ScopedWorldData& worldData = GetScopedWorldData(world);

    worldData.PreUpdateScheduler = std::make_unique<JobScheduler>("ECS.PreUpdateScheduler");
    worldData.UpdateScheduler = std::make_unique<JobScheduler>("ECS.UpdateScheduler");
    worldData.PostUpdateScheduler = std::make_unique<JobScheduler>("ECS.PostUpdateScheduler");

    // Register ECS's own PreUpdate sort pipeline BEFORE system initialization so that
    // systems can call GetPreUpdateSortJobHandle and declare dependencies on it.
    FeatureECSDynamicBlock& dynamicBlock = world.GetBlockRef<FeatureECSDynamicBlock>();
    FeatureECSScratchBlock& scratchBlock = world.GetBlockRef<FeatureECSScratchBlock>();

    auto prePartitionOwned = std::make_unique<FeatureECSDetail::PrePartitionSortedEntitiesTask>();
    auto populateOwned     = std::make_unique<FeatureECSDetail::PopulateSortedEntitiesJob>();
    auto sortOwned         = std::make_unique<FeatureECSDetail::SortEntitiesTask>();

    prePartitionOwned->ScratchBlock = &scratchBlock;
    populateOwned->DynamicBlock = &dynamicBlock;
    populateOwned->ScratchBlock = &scratchBlock;
    sortOwned->DynamicBlock     = &dynamicBlock;
    sortOwned->ScratchBlock     = &scratchBlock;

    auto* prePartition = prePartitionOwned.get();

    JobHandle hPrePartition = worldData.PreUpdateScheduler->RegisterJob(prePartitionOwned.get());
    JobHandle hPopulate     = worldData.PreUpdateScheduler->RegisterJob(populateOwned.get());
    worldData.PreUpdateSortJobHandle  = worldData.PreUpdateScheduler->RegisterJob(sortOwned.get());

    OwnedJobs.push_back(std::move(prePartitionOwned));
    OwnedJobs.push_back(std::move(populateOwned));
    OwnedJobs.push_back(std::move(sortOwned));

    worldData.PreUpdateScheduler->AddDependency(hPopulate, hPrePartition);
    worldData.PreUpdateScheduler->AddDependency(worldData.PreUpdateSortJobHandle, hPopulate);

    // Initialize systems — they may call GetPreUpdateSortJobHandle to depend on the ECS sort.
    for (const std::shared_ptr<ISystem>& system : Systems)
    {
        system->OnWorldInitialize(world);
    }

    // Build all schedulers after all jobs (ECS + system) have been registered.
    BuildAllSchedulers(world, dynamicBlock.ArchetypeManager);

    // PrePartition needs a pointer to Populate's node to read its pre-built Batches.
    // Resolved after Build() since Nodes are stable post-build.
    prePartition->PopulateNode = &worldData.PreUpdateScheduler->GetNode(hPopulate);
}

void FeatureECS::OnWorldShutdown(WorldRef world)
{
    PHX_PROFILE_ZONE_SCOPED;

    for (const std::shared_ptr<ISystem>& system : Systems)
    {
        system->OnWorldShutdown(world);
    }

    WorldData.erase(world.GetId());

    TaskQueue::ReleaseTaskQueue((uint32)world.GetId());
}

void FeatureECS::OnPreWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    const FeatureECSDynamicBlock& dynamicBlock = world.GetBlockRef<FeatureECSDynamicBlock>();
    RebuildAllSchedulersIfDirty(world, dynamicBlock.ArchetypeManager);

    ExecuteScheduler(world, EJobPhase::PreUpdate);

    ApplyCommandBuffers(world);

    // Update systems
    {
        PHX_PROFILE_ZONE_SCOPED_N("OnPreWorldUpdate_Systems");

        SystemUpdateArgs systemUpdateArgs;
        systemUpdateArgs.SimTime = args.SimTime;
        systemUpdateArgs.DeltaTime = OneDivBy(Time(Time::D));

        for (const std::shared_ptr<ISystem>& system : Systems)
        {
            const char* systemName = system->GetTypeDescriptor().GetName().c_str();
            const size_t systemNameLen = std::strlen(systemName);

            PHX_PROFILE_ZONE_SCOPED;
            PHX_PROFILE_ZONE_NAME(systemName, systemNameLen);
            
            system->OnPreWorldUpdate(world, systemUpdateArgs);
        }
    }

    WorldTaskQueue::Flush(world);
}

void FeatureECS::OnWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    ExecuteScheduler(world, EJobPhase::Update);
    ApplyCommandBuffers(world);

    // Update systems
    {
        PHX_PROFILE_ZONE_SCOPED_N("OnWorldUpdate_Systems");

        SystemUpdateArgs systemUpdateArgs;
        systemUpdateArgs.SimTime = args.SimTime;
        systemUpdateArgs.DeltaTime = OneDivBy(Time(Time::D));

        for (const std::shared_ptr<ISystem>& system : Systems)
        {
            const char* systemName = system->GetTypeDescriptor().GetName().c_str();
            const size_t systemNameLen = std::strlen(systemName);

            PHX_PROFILE_ZONE_SCOPED;
            PHX_PROFILE_ZONE_NAME(systemName, systemNameLen);

            system->OnWorldUpdate(world, systemUpdateArgs);
        }
    }

    WorldTaskQueue::Flush(world);
}

void FeatureECS::OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    ExecuteScheduler(world, EJobPhase::PostUpdate);
    ApplyCommandBuffers(world);

    // Update systems
    {
        PHX_PROFILE_ZONE_SCOPED_N("OnPostWorldUpdate_Systems");

        SystemUpdateArgs systemUpdateArgs;
        systemUpdateArgs.SimTime = args.SimTime;
        systemUpdateArgs.DeltaTime = OneDivBy(Time(Time::D));

        for (const std::shared_ptr<ISystem>& system : Systems)
        {
            const char* systemName = system->GetTypeDescriptor().GetName().c_str();
            const size_t systemNameLen = std::strlen(systemName);

            PHX_PROFILE_ZONE_SCOPED;
            PHX_PROFILE_ZONE_NAME(systemName, systemNameLen);

            system->OnPostWorldUpdate(world, systemUpdateArgs);
        }
    }


    WorldTaskQueue::Flush(world);

    SortAndCompact(world);
}

bool FeatureECS::OnPreHandleWorldAction(WorldRef world, const FeatureActionArgs& action)
{
    PHX_PROFILE_ZONE_SCOPED;

    SystemActionArgs systemActionArgs;
    systemActionArgs.SimTime = action.SimTime;
    systemActionArgs.Action = action.Action;

    for (const std::shared_ptr<ISystem>& system : Systems)
    {
        if (system->OnPreHandleWorldAction(world, systemActionArgs))
        {
            return true;
        }
    }
    
    return false;
}

bool FeatureECS::OnHandleWorldAction(WorldRef world, const FeatureActionArgs& action)
{
    PHX_PROFILE_ZONE_SCOPED;

    if (action.Action.Verb == "release_entities_in_range"_n)
    {
        Vec2 pos = { action.Action.Args[0].AsDistance, action.Action.Args[1].AsDistance };
        Distance range = action.Action.Args[2].AsDistance;

        std::vector<EntityTransform> outEntities;
        QueryEntitiesInRange(world, pos, range, outEntities);

        for (const EntityTransform& entity : outEntities)
        {
            StaticReleaseEntity(world, entity.EntityId);
        }

        return true;
    }

    SystemActionArgs systemActionArgs;
    systemActionArgs.SimTime = action.SimTime;
    systemActionArgs.Action = action.Action;

    for (const std::shared_ptr<ISystem>& system : Systems)
    {
        if (system->OnHandleWorldAction(world, systemActionArgs))
        {
            return true;
        }
    }
    
    return false;
}

bool FeatureECS::OnPostHandleWorldAction(WorldRef world, const FeatureActionArgs& action)
{
    PHX_PROFILE_ZONE_SCOPED;

    SystemActionArgs systemActionArgs;
    systemActionArgs.SimTime = action.SimTime;
    systemActionArgs.Action = action.Action;

    for (const std::shared_ptr<ISystem>& system : Systems)
    {
        if (system->OnPostHandleWorldAction(world, systemActionArgs))
        {
            return true;
        }
    }
    
    return false;
}

void FeatureECS::OnDebugRender(WorldConstRef world, const IDebugState& state, IDebugRenderer& renderer)
{
    for (const std::shared_ptr<ISystem>& system : Systems)
    {
        system->OnDebugRender(world, state, renderer);
    }
}


void FeatureECS::SortAndCompact(WorldRef world)
{
    FeatureECSDynamicBlock& dynamicBlock = world.GetBlockRef<FeatureECSDynamicBlock>();

    // TODO (jfarris): parallelize?

    {
        PHX_PROFILE_ZONE_SCOPED_N("ReclaimEntities");
        std::shared_ptr<FeatureECS> feature = GetFeature<FeatureECS>(world);
        dynamicBlock.Entities.ReclaimEntities([&](const EntityId& entityId)
        {
            feature->OnReclaimEntity(world, entityId);
        });
    }

    {
        PHX_PROFILE_ZONE_SCOPED_N("SortTags");
        dynamicBlock.Tags.Sort();
    }

    {
        PHX_PROFILE_ZONE_SCOPED_N("SortGroups");
        dynamicBlock.Groups.Sort();
    }

    {
        PHX_PROFILE_ZONE_SCOPED_N("CompactArchetypeManager");
        dynamicBlock.ArchetypeManager.Compact();
    }
}

void FeatureECS::OnReclaimEntity(WorldRef world, const EntityId& entityId) const
{
    FeatureECSDynamicBlock& block = world.GetBlockRef<FeatureECSDynamicBlock>();

    // Finish and deallocate any tasks associated with the entity
    Tasks::FeatureTask::FinishAllTasks(world, entityId);

    // If the entity has an archetype then release it now
    block.ArchetypeManager.Release(entityId);

    // Remove all associated tags
    RemoveAllTags(world, entityId);

    // Remove the entity from any groups it belongs to
    RemoveEntityFromAllGroups(world, entityId);

    // Clear its own group
    ClearGroup(world, entityId);

    // Remove all blackboard keys associated with the entity
    BlackboardQuery query(IgnoreKey, entityId, IgnoreType);
    FeatureBlackboard::GetBlackboard(world).RemoveAll(query);

    EntityDestroyedEvent.Broadcast(world, entityId);
}
void FeatureECS::ApplyCommandBuffers(WorldRef world)
{
    PHX_PROFILE_ZONE_SCOPED;

    for (const std::unique_ptr<CommandBuffer>& cb : CommandBuffers)
    {
        if (!cb || cb->IsEmpty())
        {
            continue;
        }
        for (CommandBuffer::Command cmd : *cb)
        {
            auto it = CommandHandlers.find(cmd.Id);
            PHX_ASSERT(it != CommandHandlers.end());
            if (it != CommandHandlers.end())
            {
                it->second(world, cmd);
            }
        }
        cb->Reset();
    }
}

JobScheduler& FeatureECS::GetMutableScheduler(WorldConstRef world, EJobPhase phase)
{
    ScopedWorldData& worldData = GetScopedWorldData(world);
    switch (phase)
    {
        case EJobPhase::PreUpdate:  return *worldData.PreUpdateScheduler;
        case EJobPhase::Update:     return *worldData.UpdateScheduler;
        case EJobPhase::PostUpdate: return *worldData.PostUpdateScheduler;
    }
    return *worldData.UpdateScheduler;
}

void FeatureECS::BuildAllSchedulers(WorldRef world, const ArchetypeManager& archetypes)
{
    PHX_PROFILE_ZONE_SCOPED;

    ScopedWorldData& worldData = GetScopedWorldData(world);
    worldData.PreUpdateScheduler->Build(archetypes);
    worldData.UpdateScheduler->Build(archetypes);
    worldData.PostUpdateScheduler->Build(archetypes);
}

void FeatureECS::RebuildAllSchedulersIfDirty(WorldRef world, const ArchetypeManager& archetypes)
{
    PHX_PROFILE_ZONE_SCOPED;

    ScopedWorldData& worldData = GetScopedWorldData(world);
    worldData.PreUpdateScheduler->RebuildBatchesIfDirty(archetypes);
    worldData.UpdateScheduler->RebuildBatchesIfDirty(archetypes);
    worldData.PostUpdateScheduler->RebuildBatchesIfDirty(archetypes);
}

void FeatureECS::ExecuteScheduler(WorldRef world, EJobPhase phase)
{
    PHX_PROFILE_ZONE_SCOPED;

    JobScheduler& scheduler = GetMutableScheduler(world, phase);

    std::vector<CommandBuffer*> threadCbs;
    threadCbs.reserve(CommandBuffers.size());
    for (const std::unique_ptr<CommandBuffer>& cb : CommandBuffers)
        threadCbs.push_back(cb.get());

    if (bAllowParallelJobs)
    {
        std::shared_ptr<TaskQueue> taskQueue = TaskQueue::GetTaskQueue((uint32)world.GetId());
        scheduler.Execute(world, *taskQueue, threadCbs);
    }
    else
    {
        scheduler.ExecuteSerial(world, threadCbs);
    }
}

void FeatureECS::RegisterECSCommandHandlers()
{
    CommandHandlers[Commands::AcquireEntity::StaticId] = [](WorldRef w, const CommandBuffer::Command& cmd)
    {
        const auto* data = static_cast<const Commands::AcquireEntity*>(cmd.Data);
        StaticAcquireEntity(w, data->Kind);
    };

    CommandHandlers[Commands::ReleaseEntity::StaticId] = [](WorldRef w, const CommandBuffer::Command& cmd)
    {
        const auto* data = static_cast<const Commands::ReleaseEntity*>(cmd.Data);
        StaticReleaseEntity(w, data->Target);
    };

    CommandHandlers[Commands::SetEntityKind::StaticId] = [](WorldRef w, const CommandBuffer::Command& cmd)
    {
        const auto* data = static_cast<const Commands::SetEntityKind*>(cmd.Data);
        SetEntityKind(w, data->Target, data->Kind);
    };

    CommandHandlers[Commands::AddComponentBase::StaticId] = [](WorldRef w, const CommandBuffer::Command& cmd)
    {
        const auto* data = static_cast<const Commands::AddComponentBase*>(cmd.Data);
        const void* componentPtr = data->GetComponentPtr();
        AddComponent(w, data->Target, data->ComponentType, componentPtr);
    };

    CommandHandlers[Commands::RemoveComponent::StaticId] = [](WorldRef w, const CommandBuffer::Command& cmd)
    {
        const auto* data = static_cast<const Commands::RemoveComponent*>(cmd.Data);
        RemoveComponent(w, data->Target, data->ComponentType);
    };

    CommandHandlers[Commands::AddTag::StaticId] = [](WorldRef w, const CommandBuffer::Command& cmd)
    {
        const auto* data = static_cast<const Commands::AddTag*>(cmd.Data);
        AddTag(w, data->Target, data->Tag);
    };

    CommandHandlers[Commands::RemoveTag::StaticId] = [](WorldRef w, const CommandBuffer::Command& cmd)
    {
        const auto* data = static_cast<const Commands::RemoveTag*>(cmd.Data);
        RemoveTag(w, data->Target, data->Tag);
    };

    CommandHandlers[Commands::RemoveAllTags::StaticId] = [](WorldRef w, const CommandBuffer::Command& cmd)
    {
        const auto* data = static_cast<const Commands::RemoveAllTags*>(cmd.Data);
        RemoveAllTags(w, data->Target);
    };

    CommandHandlers[Commands::AddEntityToGroup::StaticId] = [](WorldRef w, const CommandBuffer::Command& cmd)
    {
        const auto* data = static_cast<const Commands::AddEntityToGroup*>(cmd.Data);
        AddEntityToGroup(w, data->Group, data->EntityToAdd);
    };

    CommandHandlers[Commands::RemoveEntityFromGroup::StaticId] = [](WorldRef w, const CommandBuffer::Command& cmd)
    {
        const auto* data = static_cast<const Commands::RemoveEntityFromGroup*>(cmd.Data);
        RemoveEntityFromGroup(w, data->Group, data->EntityToRemove);
    };

    CommandHandlers[Commands::RemoveEntityFromAllGroups::StaticId] = [](WorldRef w, const CommandBuffer::Command& cmd)
    {
        const auto* data = static_cast<const Commands::RemoveEntityFromAllGroups*>(cmd.Data);
        RemoveEntityFromAllGroups(w, data->Target);
    };

    CommandHandlers[Commands::ClearGroup::StaticId] = [](WorldRef w, const CommandBuffer::Command& cmd)
    {
        const auto* data = static_cast<const Commands::ClearGroup*>(cmd.Data);
        ClearGroup(w, data->Target);
    };

    CommandHandlers[Commands::SetBlackboardValueBase::StaticId] = [](WorldRef w, const CommandBuffer::Command& cmd)
    {
        const auto* data = static_cast<const Commands::SetBlackboardValueBase*>(cmd.Data);
        const void* valuePtr = data->GetValuePtr();
        SetBlackboardValueRaw(w, data->Target, data->Key, *static_cast<const blackboard_value_t*>(valuePtr));
    };
}

struct FeatureECS::ScopedWorldData& FeatureECS::GetScopedWorldData(WorldConstRef world)
{
    return WorldData[world.GetId()];
}
