
#include "PhoenixSim/ECS/FeatureECS.h"

#include <execution>

#include "PhoenixSim/MortonCode.h"
#include "PhoenixSim/Profiling.h"
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

FeatureECS::FeatureECS()
{
}

FeatureECS::FeatureECS(const FeatureECSCtorArgs& args)
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

        TArray<EntityTransform> outEntities;
        QueryEntitiesInRange(world, pos, range, outEntities);

        for (const EntityTransform& entity : outEntities)
        {
            ReleaseEntity(world, entity.EntityId);
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

const TArray<TSharedPtr<ISystem>>& FeatureECS::GetSystems() const
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

EntityId FeatureECS::AcquireEntity(WorldRef world, const FName& kind)
{
    PHX_PROFILE_ZONE_SCOPED;

    FeatureECSDynamicBlock& block = world.GetBlockRef<FeatureECSDynamicBlock>();

    EntityId entityId = block.Entities.Acquire(kind);
    if (entityId == EntityId::Invalid)
    {
        return EntityId::Invalid;
    }

    Entity& entity = block.Entities.GetEntityRef(entityId);

    // Automatically acquire an archetype if the kind matches one
    if (block.ArchetypeManager.IsArchetypeRegistered(kind))
    {
        entity.Handle = block.ArchetypeManager.Acquire(entityId, kind);
    }

    return entityId;
}

bool FeatureECS::ReleaseEntity(WorldRef world, EntityId entityId)
{
    FeatureECSDynamicBlock& block = world.GetBlockRef<FeatureECSDynamicBlock>();

    if (!block.Entities.IsValid(entityId))
    {
        return false;
    }

    Entity& entity = block.Entities.GetEntityRef(entityId);

    // If the entity has an archetype then release it now
    block.ArchetypeManager.Release(entity.Handle);

    // Remove all associated tags
    RemoveAllTags(world, entityId);

    // Remove the entity from any groups it belongs to
    RemoveEntityFromAllGroups(world, entityId);

    // Clear its own group
    ClearGroup(world, entityId);

    // Remove all blackboard keys associated with the entity
    BlackboardKeyQuery query(IgnoreKey, entityId, IgnoreType);
    FeatureBlackboard::GetBlackboard(world).RemoveAll(query);

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
    (void)block.ArchetypeManager.Release(entity->Handle);

    // Automatically acquire an archetype if the kind matches one
    if (block.ArchetypeManager.IsArchetypeRegistered(kind))
    {
        entity->Handle = block.ArchetypeManager.Acquire(entity->GetId(), kind);
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

    Entity* entity = GetEntityPtr(world, entityId);
    if (!entity)
    {
        return nullptr;
    }

    return static_cast<IComponent*>(block->ArchetypeManager.GetComponent(entity->Handle, componentType));
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

    const Entity* entity = GetEntityPtr(world, entityId);
    if (!entity)
    {
        return nullptr;
    }

    return static_cast<const IComponent*>(block->ArchetypeManager.GetComponent(entity->Handle, componentType));
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

    return static_cast<IComponent*>(block->ArchetypeManager.AddComponent(entity->Handle, componentType));
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

    return block->ArchetypeManager.RemoveComponent(entity->Handle, componentType);
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

    return block->ArchetypeManager.RemoveAllComponents(entity->Handle);
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

blackboard_key_t FeatureECS::CreateBlackboardKey(
    const EntityId& id,
    const FName& key,
    blackboard_type_t type)
{
    return BlackboardKey::Create(static_cast<uint32>(key), id, type);
}

bool FeatureECS::HasBlackboardValue(
    WorldConstRef world,
    const EntityId& id,
    const FName& key,
    blackboard_type_t type)
{
    const WorldBlackboard& blackboard = FeatureBlackboard::GetBlackboard(world);
    blackboard_key_t fullKey = CreateBlackboardKey(id, key);
    return blackboard.HasValue(BlackboardKeyQuery(fullKey, type));
}

bool FeatureECS::SetBlackboardValueRaw(
    WorldRef world,
    const EntityId& id,
    const FName& key,
    blackboard_value_t value,
    blackboard_type_t type)
{
    WorldBlackboard& blackboard = FeatureBlackboard::GetBlackboard(world);
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
    const WorldBlackboard& blackboard = FeatureBlackboard::GetBlackboard(world);
    blackboard_key_t fullKey = CreateBlackboardKey(id, key);
    return blackboard.GetValue(BlackboardKeyQuery(fullKey, expectedType), outValue);
}

blackboard_value_t FeatureECS::GetBlackboardValueRaw(
    WorldConstRef world,
    const EntityId& id,
    const FName& key,
    blackboard_type_t expectedType)
{
    const WorldBlackboard& blackboard = FeatureBlackboard::GetBlackboard(world);
    blackboard_key_t fullKey = CreateBlackboardKey(id, key);
    blackboard_value_t value;
    blackboard.GetValue(BlackboardKeyQuery(fullKey, expectedType), value);
    return value;
}

const Transform2D* FeatureECS::GetLocalTransformPtr(WorldConstRef world, EntityId entityId)
{
    const TransformComponent* comp = GetComponent<TransformComponent>(world, entityId);
    return comp ? &comp->Transform : nullptr;
}

const Transform2D* FeatureECS::GetWorldTransformPtr(WorldConstRef world, EntityId entityId)
{
    const TransformComponent* comp = GetComponent<TransformComponent>(world, entityId);
    return comp ? &comp->Transform : nullptr;
}

Vec2 FeatureECS::GetWorldPosition(WorldConstRef world, EntityId entityId)
{
    const TransformComponent* comp = GetComponent<TransformComponent>(world, entityId);
    return comp ? comp->Transform.Position : Vec2::Zero;
}

bool FeatureECS::IsInRange(WorldConstRef world, EntityId entity, EntityId target, Distance range)
{
    auto targetTransform = GetWorldTransformPtr(world, target);
    if (!targetTransform)
    {
        return false;
    }

    return IsInRange(world, entity, targetTransform->Position, range);
}

bool FeatureECS::IsInRange(WorldConstRef world, EntityId entity, const Vec2& target, Distance range)
{
    auto entityTransform = GetWorldTransformPtr(world, entity);
    if (!entityTransform)
    {
        return false;
    }

    return Vec2::Distance(entityTransform->Position, target) <= range;
}

bool FeatureECS::IsFacing(WorldConstRef world, EntityId entity, EntityId target, Angle threshold)
{
    auto targetTransform = GetWorldTransformPtr(world, target);
    if (!targetTransform)
    {
        return false;
    }

    return IsFacing(world, entity, targetTransform->Position, threshold);
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

    return Abs(AngleDelta(entityFacing, targetFacing)) <= threshold;
}

void FeatureECS::QueryEntitiesInRange(
    WorldConstRef world,
    const Vec2& pos,
    Distance range,
    TArray<EntityTransform>& outEntities,
    const EntityRangeQueryArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    const FeatureECSScratchBlock* scratchBlock = world.GetBlock<FeatureECSScratchBlock>();
    if (!scratchBlock)
    {
        return;
    }

    // Query for overlapping morton ranges
    TMortonCodeRangeArray ranges;
    MortonCodeAABB aabb = ToMortonCodeAABB(pos, range);
    MortonCodeQuery(aabb, ranges);

    TArray<EntityTransform*> overlappingEntities;
    ForEachInMortonCodeRanges<EntityTransform, &EntityTransform::ZCode>(
        scratchBlock->SortedEntities,
        ranges,
        [&](const EntityTransform& entityTransform)
        {
            const Entity* entity = GetEntityPtr(world, entityTransform.EntityId);
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
    TArray<EntityTransform>& outEntities,
    const EntityRangeQueryArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    const FeatureECSScratchBlock* scratchBlock = world.GetBlock<FeatureECSScratchBlock>();
    if (!scratchBlock)
    {
        return;
    }

    // Query for overlapping morton ranges
    TMortonCodeRangeArray ranges;
    MortonCodeAABB aabb = ToMortonCodeAABB(min, max);
    MortonCodeQuery(aabb, ranges);

    TArray<EntityTransform*> overlappingEntities;
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
    dynamicBlock.ArchetypeManager.Compact();

    {
        PHX_PROFILE_ZONE_SCOPED_N("SortTags");
        dynamicBlock.Tags.Sort();
    }

    {
        PHX_PROFILE_ZONE_SCOPED_N("SortGroups");
        dynamicBlock.Groups.Sort();
    }
}
