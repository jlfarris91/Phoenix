
#include "PhoenixSim/ECS/FeatureECS.h"

#include <execution>

#include "PhoenixSim/MortonCode.h"
#include "PhoenixSim/Profiling.h"
#include "PhoenixSim/Session.h"
#include "PhoenixSim/WorldTaskQueue.h"
#include "PhoenixSim/ECS/System.h"
#include "PhoenixSim/ECS/SystemJob.h"
#include "PhoenixSim/Blackboard/FeatureBlackboard.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::Blackboard;

namespace FeatureECSDetail
{
    struct PopulateSortedEntitiesJob : IBufferJob<TransformComponent&>
    {
        void Execute(const EntityComponentSpan<TransformComponent&>& span) override
        {
            PHX_PROFILE_ZONE_SCOPED_N("PopulateSortedEntitiesJob");

            FeatureECSScratchBlock& scratchBlock = World->GetBlockRef<FeatureECSScratchBlock>();

            for (auto && [entityId, index, transformComp] : span)
            {
                transformComp.ZCode = ToMortonCode(transformComp.Transform.Position);

                uint32 sortedEntityIndex = scratchBlock.SortedEntityCount.fetch_add(1);
                scratchBlock.SortedEntities[sortedEntityIndex] = EntityTransform(entityId, &transformComp, transformComp.ZCode);
            }
        }
    };

    void SortEntitiesByZCodeTask(WorldRef world)
    {    
        PHX_PROFILE_ZONE_SCOPED;

        FeatureECSScratchBlock& scratchBlock = world.GetBlockRef<FeatureECSScratchBlock>();

        // Calculated from PopulateSortedEntitiesJob
        scratchBlock.SortedEntities.SetSize(scratchBlock.SortedEntityCount);

        std::sort(
            std::execution::par,
            scratchBlock.SortedEntities.begin(),
            scratchBlock.SortedEntities.end(),
            [](const EntityTransform& a, const EntityTransform& b)
            {
                return a.ZCode < b.ZCode;
            });
    }
}

FeatureECSDynamicBlock::FeatureECSDynamicBlock(BlockBufferAllocator& allocator, const Config& config)
    : Entities(allocator, config.MaxEntities)
    , Tags(allocator, config.MaxTags)
    , Groups(allocator, config.MaxGroups)
    , ArchetypeManager(allocator, config.ArchetypeManager)
{
}

FeatureECSDynamicBlock::FeatureECSDynamicBlock(
    BlockBufferAllocator& allocator,
    const Config& config,
    const FeatureECSDynamicBlock& other)
    : Entities(allocator, config.MaxEntities, other.Entities)
    , Tags(allocator, config.MaxTags, other.Tags)
    , Groups(allocator, config.MaxGroups, other.Groups)
    , ArchetypeManager(allocator, config.ArchetypeManager, other.ArchetypeManager)
{
}

BufferBlockLayout FeatureECSDynamicBlock::Layout(Config config)
{
    BufferBlockLayout layout;
    layout.BlockSize = sizeof(FeatureECSDynamicBlock);
    layout.AllocSize += FixedEntityList::GetAllocSizeBytes(config.MaxEntities);
    layout.AllocSize += FixedTagList::GetAllocSizeBytes(config.MaxTags);
    layout.AllocSize += FixedGroupList::GetAllocSizeBytes(config.MaxGroups);
    layout.AllocSize += ArchetypeManager::GetAllocSizeBytes(config.ArchetypeManager);
    return layout;
}

void FeatureECSDynamicBlock::Construct(void* dest, BlockBufferAllocator& allocator, Config config)
{
    new (dest) FeatureECSDynamicBlock(allocator, config);
}

FeatureECSScratchBlock::FeatureECSScratchBlock(BlockBufferAllocator& allocator, const Config& config)
    : SortedEntities(allocator, config.MaxEntities)
{
}

FeatureECSScratchBlock::FeatureECSScratchBlock(
    BlockBufferAllocator& allocator,
    const Config& config,
    const FeatureECSScratchBlock& other)
    : SortedEntities(allocator, config.MaxEntities, other.SortedEntities)
    , SortedEntityCount(other.SortedEntityCount.load())
{
}

BufferBlockLayout FeatureECSScratchBlock::Layout(Config config)
{
    BufferBlockLayout layout;
    layout.BlockSize = sizeof(FeatureECSScratchBlock);
    layout.AllocSize += TFixedArray<EntityTransform>::GetAllocSizeBytes(config.MaxEntities);
    return layout;
}

void FeatureECSScratchBlock::Construct(void* dest, BlockBufferAllocator& allocator, Config config)
{
    new (dest) FeatureECSScratchBlock(allocator, config);
}

FeatureECS::FeatureECS()
{
    FEATURE_CHANNEL(FeatureChannels::WorldInitialize)
    FEATURE_CHANNEL(FeatureChannels::WorldShutdown)
    FEATURE_CHANNEL(FeatureChannels::PreWorldUpdate)
    FEATURE_CHANNEL(FeatureChannels::WorldUpdate)
    FEATURE_CHANNEL(FeatureChannels::PostWorldUpdate)
    FEATURE_CHANNEL(FeatureChannels::PreHandleWorldAction)
    FEATURE_CHANNEL(FeatureChannels::HandleWorldAction)
    FEATURE_CHANNEL(FeatureChannels::PostHandleWorldAction)
    FEATURE_CHANNEL(FeatureChannels::DebugRender)
}

FeatureECS::FeatureECS(const FeatureECSCtorArgs& args)
    : FeatureECS()
{
    for (const TSharedPtr<ISystem>& system : args.Systems)
    {
        Systems.push_back(system);
    }
}

void FeatureECS::OnPreUpdate(const FeatureUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    SystemUpdateArgs systemUpdateArgs;
    systemUpdateArgs.SimTime = args.SimTime;
    systemUpdateArgs.DeltaTime = OneDivBy(Time(args.StepHz));

    for (const TSharedPtr<ISystem>& system : Systems)
    {
        system->OnPreUpdate(systemUpdateArgs);
    }
}

void FeatureECS::OnUpdate(const FeatureUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    SystemUpdateArgs systemUpdateArgs;
    systemUpdateArgs.SimTime = args.SimTime;
    systemUpdateArgs.DeltaTime = OneDivBy(Time(args.StepHz));

    for (const TSharedPtr<ISystem>& system : Systems)
    {
        system->OnUpdate(systemUpdateArgs);
    }
}

void FeatureECS::OnPostUpdate(const FeatureUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    SystemUpdateArgs systemUpdateArgs;
    systemUpdateArgs.SimTime = args.SimTime;
    systemUpdateArgs.DeltaTime = OneDivBy(Time(args.StepHz));

    for (const TSharedPtr<ISystem>& system : Systems)
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

    for (const TSharedPtr<ISystem>& system : Systems)
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

    for (const TSharedPtr<ISystem>& system : Systems)
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

    for (const TSharedPtr<ISystem>& system : Systems)
    {
        if (system->OnPostHandleAction(systemActionArgs))
        {
            return true;
        }
    }
    
    return false;
}

void FeatureECS::OnWorldLayout(const WorldLayoutContext& context, WorldLayoutBuilder& builder)
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

    if (const FeatureJsonConfig* featureConfig = context.Config.GetFeatureConfig(StaticTypeName))
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

    for (const TSharedPtr<ISystem>& system : Systems)
    {
        system->OnWorldInitialize(world);
    }
}

void FeatureECS::OnWorldShutdown(WorldRef world)
{
    PHX_PROFILE_ZONE_SCOPED;

    for (const TSharedPtr<ISystem>& system : Systems)
    {
        system->OnWorldShutdown(world);
    }
    
    TaskQueue::ReleaseTaskQueue((uint32)world.GetId());
}

bool FeatureECS::InitView(WorldConstRef world, ViewContext& context)
{
    return IFeature::InitView(world, context);
}

void FeatureECS::FillView(WorldConstRef world, const ViewContext& context)
{
    IFeature::FillView(world, context);
}

void FeatureECS::OnPreWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    SortEntitiesByZCode(world);

    SystemUpdateArgs systemUpdateArgs;
    systemUpdateArgs.SimTime = args.SimTime;
    systemUpdateArgs.DeltaTime = OneDivBy(Time(args.StepHz));

    for (const TSharedPtr<ISystem>& system : Systems)
    {
        system->OnPreWorldUpdate(world, systemUpdateArgs);
    }

    WorldTaskQueue::Flush(world);
}

void FeatureECS::OnWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    SystemUpdateArgs systemUpdateArgs;
    systemUpdateArgs.SimTime = args.SimTime;
    systemUpdateArgs.DeltaTime = OneDivBy(Time(args.StepHz));
    
    for (const TSharedPtr<ISystem>& system : Systems)
    {
        system->OnWorldUpdate(world, systemUpdateArgs);
    }

    WorldTaskQueue::Flush(world);
}

void FeatureECS::OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    SystemUpdateArgs systemUpdateArgs;
    systemUpdateArgs.SimTime = args.SimTime;
    systemUpdateArgs.DeltaTime = OneDivBy(Time(args.StepHz));
    
    for (const TSharedPtr<ISystem>& system : Systems)
    {
        system->OnPostWorldUpdate(world, systemUpdateArgs);
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

    for (const TSharedPtr<ISystem>& system : Systems)
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
        Vec2 pos = { action.Action.Data[0].Distance, action.Action.Data[1].Distance };
        Distance range = action.Action.Data[2].Distance;

        TVector<EntityTransform> outEntities;
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

    for (const TSharedPtr<ISystem>& system : Systems)
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

    for (const TSharedPtr<ISystem>& system : Systems)
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
    for (const TSharedPtr<ISystem>& system : Systems)
    {
        system->OnDebugRender(world, state, renderer);
    }
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

void FeatureECS::RegisterSystem(const TSharedPtr<ISystem>& system)
{
    Systems.push_back(system);
}

bool FeatureECS::UnregisterSystem(const TSharedPtr<ISystem>& system)
{
    auto iter = std::ranges::find(Systems, system);
    if (iter == Systems.end())
        return false;
    Systems.erase(iter);
    return true;
}

const TVector<TSharedPtr<ISystem>>& FeatureECS::GetSystems() const
{
    return Systems;
}

const decltype(FeatureECSDynamicBlock::Entities)* FeatureECS::GetEntities(WorldConstRef world)
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
    TSharedPtr<FeatureECS> feature = GetFeature<FeatureECS>(world);
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
    TSharedPtr<FeatureECS> feature = GetFeature<FeatureECS>(world);
    return feature && feature->ReleaseEntity(world, entityId);
}

bool FeatureECS::ReleaseEntity(WorldRef world, EntityId entityId) const
{
    FeatureECSDynamicBlock& block = world.GetBlockRef<FeatureECSDynamicBlock>();

    if (!block.Entities.IsValid(entityId))
    {
        return false;
    }

    block.Entities.Release(entityId);
    EntityReleasedEvent.Broadcast(world, entityId);

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
    const FName& componentType)
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

    return static_cast<IComponent*>(block->ArchetypeManager.AddComponent(entity->Id, componentType));
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

Vec2 FeatureECS::GetWorldPosition(WorldConstRef world, EntityId entityId)
{
    auto entityTransform = GetWorldTransformPtr(world, entityId);
    return entityTransform ? entityTransform->Position : Vec2::Zero;
}

Angle FeatureECS::GetWorldFacing(WorldConstRef world, EntityId entityId)
{
    auto entityTransform = GetWorldTransformPtr(world, entityId);
    return entityTransform ? entityTransform->Rotation : 0;
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
    TVector<EntityTransform>& outEntities,
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

    TVector<EntityTransform*> overlappingEntities;
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
    TVector<EntityTransform>& outEntities,
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

    TVector<EntityTransform*> overlappingEntities;
    ForEachInMortonCodeRanges<EntityTransform, &EntityTransform::ZCode>(
        scratchBlock->SortedEntities,
        ranges,
        [&](const EntityTransform& entityBody)
        {
            outEntities.push_back(entityBody);
        });
}

void FeatureECS::SortEntitiesByZCode(WorldRef world)
{
    PHX_PROFILE_ZONE_SCOPED;
    
    FeatureECSScratchBlock& scratchBlock = world.GetBlockRef<FeatureECSScratchBlock>();
    
    scratchBlock.SortedEntities.Reset();
    scratchBlock.SortedEntityCount = 0;
    
    // Calculate z-codes in parallel
    FeatureECSDetail::PopulateSortedEntitiesJob job;
    ScheduleParallel(world, job);

    WorldTaskQueue::Schedule(world, &FeatureECSDetail::SortEntitiesByZCodeTask);
}

void FeatureECS::SortAndCompact(WorldRef world)
{
    FeatureECSDynamicBlock& dynamicBlock = world.GetBlockRef<FeatureECSDynamicBlock>();

    // TODO (jfarris): parallelize?

    {
        PHX_PROFILE_ZONE_SCOPED_N("ReclaimEntities");
        TSharedPtr<FeatureECS> feature = GetFeature<FeatureECS>(world);
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
