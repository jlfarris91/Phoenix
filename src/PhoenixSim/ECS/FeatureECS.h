
#pragma once

#include <unordered_map>

#include "PhoenixSim/Delegates.h"
#include "PhoenixSim/Features.h"
#include "PhoenixSim/Parallel.h"
#include "PhoenixSim/Worlds.h"
#include "PhoenixSim/Blackboard/FeatureBlackboard.h"
#include "PhoenixSim/Blackboard/FixedBlackboard.h"
#include "PhoenixSim/ECS/ArchetypeManager.h"
#include "PhoenixSim/ECS/CommandBuffer.h"
#include "PhoenixSim/ECS/Entity.h"
#include "PhoenixSim/ECS/EntityId.h"
#include "PhoenixSim/ECS/FixedEntityList.h"
#include "PhoenixSim/ECS/FixedGroupList.h"
#include "PhoenixSim/ECS/FixedTagList.h"
#include "PhoenixSim/ECS/JobScheduler.h"
#include "PhoenixSim/ECS/System.h"
#include "PhoenixSim/ECS/TransformComponent.h"

#ifndef PHX_ECS_MAX_ENTITIES
#define PHX_ECS_MAX_ENTITIES 32768
#endif

#ifndef PHX_ECS_MAX_TAGS
#define PHX_ECS_MAX_TAGS (PHX_ECS_MAX_ENTITIES << 1)
#endif

#ifndef PHX_ECS_MAX_GROUPS
#define PHX_ECS_MAX_GROUPS (PHX_ECS_MAX_ENTITIES << 1)
#endif

#ifndef PHX_ECS_MAX_COMPONENT_DEFS
#define PHX_ECS_MAX_COMPONENT_DEFS 256
#endif

#ifndef PHX_ECS_MAX_ARCHETYPE_DEFS
#define PHX_ECS_MAX_ARCHETYPE_DEFS 256
#endif

#ifndef PHX_ECS_MAX_ARCHETYPE_LISTS
#define PHX_ECS_MAX_ARCHETYPE_LISTS 1024
#endif

#ifndef PHX_ECS_ARCHETYPE_LIST_SIZE
#define PHX_ECS_ARCHETYPE_LIST_SIZE 16000
#endif

namespace Phoenix::ECS
{
    class ISystem;
    struct EntityQuery;

    struct PHOENIX_SIM_API FeatureECSDynamicBlock : BlockBufferBlock
    {
        PHX_DECLARE_BLOCK_WITH_ALLOC(FeatureECSDynamicBlock)
        {
            uint32 MaxEntities = 0;
            uint32 MaxTags = 0;
            uint32 MaxGroups = 0;
            ArchetypeManager::Config ArchetypeManager;
        };

        FixedEntityList Entities;
        ArchetypeManager ArchetypeManager;
        FixedTagList Tags;
        FixedGroupList Groups;
    };

    struct PHOENIX_SIM_API FeatureECSScratchBlock : BlockBufferBlock
    {
        PHX_DECLARE_BLOCK_WITH_ALLOC(FeatureECSScratchBlock)
        {
            uint32 MaxEntities = 0;
        };

        TFixedArray<EntityTransform> SortedEntities;
        TFixedArray<uint32> SortedEntityIndex;  // [entityId % capacity] → position in SortedEntities
        std::atomic<uint32> SortedEntityCount = 0;

        // Tracks sorted entity count from previous frame for bounded memset
        uint32 PrevSortedEntityCount = PHX_ECS_MAX_ENTITIES;
    };

    struct EntityRangeQueryArgs
    {
        TOptional<std::unordered_set<FName>> Kinds;
        TOptional<std::unordered_set<FName>> AnyComponents;
    };

    PHX_DECLARE_MULTICAST_DELEGATE(FOnEntityAcquired, WorldRef world, EntityId entityId);
    PHX_DECLARE_MULTICAST_DELEGATE(FOnEntityReleasing, WorldRef world, EntityId entityId);
    PHX_DECLARE_MULTICAST_DELEGATE(FOnEntityReleased, WorldRef world, EntityId entityId);

    class PHOENIX_SIM_API FeatureECS final : public IFeature
    {
        PHX_DECLARE_FEATURE_TYPE(FeatureECS)
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

    public:

        //
        // Events
        //

        FOnEntityAcquired& OnEntityAcquired();
        FOnEntityReleasing& OnEntityReleasing();
        FOnEntityReleased& OnEntityReleased();

        //
        // System Management
        //

        // Registers a new ECS system.
        void RegisterSystem(const std::shared_ptr<ISystem>& system);

        // Unregisters an existing ECS system. Returns true if the system was removed.
        bool UnregisterSystem(const std::shared_ptr<ISystem>& system);

        const std::vector<std::shared_ptr<ISystem>>& GetSystems() const;

        //
        // Entity Management
        //

        // Gets a pointer to the entities array for a given world.
        static const FixedEntityList* GetEntities(WorldConstRef world);
        
        static bool IsEntityValid(WorldConstRef world, EntityId entityId);
        
        static Entity* GetEntityPtr(WorldRef world, EntityId entityId);
        static const Entity* GetEntityPtr(WorldConstRef world, EntityId entityId);

        static Entity& GetEntityRef(WorldRef world, EntityId entityId);
        static const Entity& GetEntityRef(WorldConstRef world, EntityId entityId);

        static EntityId StaticAcquireEntity(WorldRef world, const FName& kind);
        EntityId AcquireEntity(WorldRef world, const FName& kind) const;

        static bool StaticReleaseEntity(WorldRef world, EntityId entityId);
        bool ReleaseEntity(WorldRef world, EntityId entityId) const;

        static bool SetEntityKind(WorldRef world, EntityId entityId, const FName& kind);

        template <class ...TComponents>
        static void ForEachEntity(WorldRef world, const EntityQuery& query, const TEntityQueryFunc<TComponents...>& func)
        {
            FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
            if (!block)
            {
                return;
            }

            return block->ArchetypeManager.ForEachEntity<TComponents...>(query, func);
        }

        template <class ...TComponents>
        static void ForEachEntity(WorldRef world, const TEntityQueryFunc<TComponents...>& func)
        {
            EntityQueryBuilder builder;
            builder.RequireAllComponents<TComponents...>();
            ForEachEntity(world, builder.GetQuery(), func);
        }

        template <class ...TComponents>
        static void ForEachEntity(WorldRef world, const EntityQuery& query, const TEntityQueryBufferFunc<TComponents...>& func)
        {
            FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
            if (!block)
            {
                return;
            }

            return block->ArchetypeManager.ForEachEntity<TComponents...>(query, func);
        }

        template <class ...TComponents>
        static void ForEachEntity(WorldRef world, const TEntityQueryBufferFunc<TComponents...>& func)
        {
            EntityQueryBuilder builder;
            builder.RequireAllComponents<TComponents...>();
            ForEachEntity(world, builder.GetQuery(), func);
        }

        template <class ...TComponents>
        static void ForEachEntity(WorldConstRef world, const EntityQuery& query, const TEntityQueryFunc<TComponents...>& func)
        {
            const FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
            if (!block)
            {
                return;
            }

            return block->ArchetypeManager.ForEachEntity<TComponents...>(query, func);
        }

        template <class ...TComponents>
        static void ForEachEntity(WorldConstRef world, const TEntityQueryFunc<TComponents...>& func)
        {
            EntityQueryBuilder builder;
            builder.RequireAllComponents<TComponents...>();
            ForEachEntity(world, builder.GetQuery(), func);
        }

        template <class ...TComponents>
        static void ForEachEntity(WorldConstRef world, const EntityQuery& query, const TEntityQueryBufferFunc<TComponents...>& func)
        {
            const FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
            if (!block)
            {
                return;
            }

            return block->ArchetypeManager.ForEachEntity<TComponents...>(query, func);
        }

        template <class ...TComponents>
        static void ForEachEntity(WorldConstRef world, const TEntityQueryBufferFunc<TComponents...>& func)
        {
            EntityQueryBuilder builder;
            builder.RequireAllComponents<TComponents...>();
            ForEachEntity(world, builder.GetQuery(), func);
        }

        template <class TJob>
        static void ForEachEntity(WorldRef world, const EntityQuery& query, TJob& job)
        {
            FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
            if (!block)
            {
                return;
            }

            return block->ArchetypeManager.ForEachEntity(world, query, job);
        }

        template <class TJob>
        static void ForEachEntity(WorldRef world, TJob& job)
        {
            ForEachEntity(world, EntityJobHelper<TJob>::BuildQuery(), job);
        }

        template <class TJob>
        static void ForEachEntity(WorldConstRef world, const EntityQuery& query, TJob& job)
        {
            const FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
            if (!block)
            {
                return;
            }

            return block->ArchetypeManager.ForEachEntity(world, query, job);
        }

        template <class TJob>
        static void ForEachEntity(WorldConstRef world, TJob& job)
        {
            ForEachEntity(world, EntityJobHelper<TJob>::BuildQuery(), job);
        }

        //
        // Archetype Management
        //

        static bool RegisterArchetypeDefinition(WorldRef world, const ArchetypeDefinition& definition);

        template <class ...TComponents>
        static bool RegisterArchetypeDefinition(WorldRef world, const FName& id = FName::None)
        {
            return RegisterArchetypeDefinition(world, ArchetypeDefinition::Create<TComponents...>(id));
        }

        static bool UnregisterArchetypeDefinition(WorldRef world, const ArchetypeDefinition& definition);

        static bool HasArchetypeDefinition(WorldConstRef world, const FName& name);

        //
        // Component Management
        //

        static bool RegisterComponentDefinition(WorldRef world, const ComponentDefinition& definition);

        template <class TComponent>
        static bool RegisterComponentDefinition(WorldRef world)
        {
            return RegisterComponentDefinition(world, ComponentDefinition::Create<TComponent>());
        }

        static bool UnregisterComponentDefinition(WorldRef world, const FName& componentType);

        template <class TComponent>
        static bool UnregisterComponentDefinition(WorldRef world)
        {
            return UnregisterComponentDefinition(world, StaticTypeName<TComponent>::TypeId);
        }

        static bool HasComponentDefinition(WorldRef world, const FName& componentType);

        // Gets the pointer to a component on an entity if it exists.
        static IComponent* GetComponent(WorldRef world, EntityId entityId, const FName& componentType);

        // Gets the pointer to a component on an entity if it exists.
        static const IComponent* GetComponent(WorldConstRef world, EntityId entityId, const FName& componentType);

        // Gets a reference to a component on an entity if it exists.
        static IComponent& GetComponentRef(WorldRef world, EntityId entityId, const FName& componentType);

        // Gets a reference to a component on an entity if it exists.
        static const IComponent& GetComponentRef(WorldConstRef world, EntityId entityId, const FName& componentType);

        // Gets the pointer to a component on an entity if it exists.
        template <class T>
        static T* GetComponent(WorldRef world, EntityId entityId)
        {
            IComponent* comp = GetComponent(world, entityId, StaticTypeName<T>::TypeId);
            return static_cast<T*>(comp);
        }

        // Gets the pointer to a component on an entity if it exists.
        template <class T>
        static const T* GetComponent(WorldConstRef world, EntityId entityId)
        {
            const IComponent* comp = GetComponent(world, entityId, StaticTypeName<T>::TypeId);
            return static_cast<const T*>(comp);
        }

        // Gets the pointer to a component on an entity or adds it if it doesn't exist.
        template <class T>
        static T* GetOrAddComponent(WorldRef world, EntityId entityId)
        {
            IComponent* comp = GetComponent(world, entityId, StaticTypeName<T>::TypeId);
            if (!comp && IsEntityValid(world, entityId))
            {
                comp = AddComponent<T>(world, entityId);
            }
            return static_cast<T*>(comp);
        }

        // Gets a reference to a component on an entity if it exists.
        template <class T>
        static T& GetComponentRef(WorldRef world, EntityId entityId)
        {
            IComponent& comp = GetComponentRef(world, entityId, StaticTypeName<T>::TypeId);
            return static_cast<T&>(comp);
        }

        // Gets a reference to a component on an entity if it exists.
        template <class T>
        static const T& GetComponentRef(WorldConstRef world, EntityId entityId)
        {
            const IComponent& comp = GetComponentRef(world, entityId, StaticTypeName<T>::TypeId);
            return static_cast<const T&>(comp);
        }

        // Returns whether an entity is using an archetype containing a type of component.
        static bool HasComponent(WorldConstRef world, EntityId entityId, const FName& componentType);

        // Adds a new component to an entity.
        static IComponent* AddComponent(
            WorldRef world,
            EntityId entityId,
            const FName& componentType,
            const void* componentData);

        // Adds a new component to an entity.
        template <class T>
        static T* AddComponent(WorldRef world, EntityId entityId, const T& defaultValue = {})
        {
            FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
            if (!block)
            {
                return nullptr;
            }

            return block->ArchetypeManager.AddComponent<T>(entityId, defaultValue);
        }

        // Adds a new component to an entity.
        template <class T, class ...TArgs>
        static T* EmplaceComponent(WorldRef world, EntityId entityId, const TArgs&...args)
        {
            FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
            if (!block)
            {
                return nullptr;
            }

            return block->ArchetypeManager.EmplaceComponent<T, TArgs...>(entityId, args...);
        }

        // Removes a component from an entity.
        static bool RemoveComponent(WorldRef world, EntityId entityId, const FName& componentType);

        template <class TComponent>
        static bool RemoveComponent(WorldRef world, EntityId entityId)
        {
            return RemoveComponent(world, entityId, StaticTypeName<TComponent>::TypeId);
        }

        // Removes all components from an entity, effectively releasing the associated archetype.
        static uint32 RemoveAllComponents(WorldRef world, EntityId entityId);

        // Enumerates all components on a given entity.
        template <class T>
        static void ForEachComponent(WorldRef world, EntityId entityId, const T& callback)
        {
            FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
            if (!block)
            {
                return;
            }

            return block->ArchetypeManager.ForEachComponent<T>(entityId, callback);
        }

        // Enumerates all components on a given entity.
        template <class T>
        static void ForEachComponent(WorldConstRef world, EntityId entityId, const T& callback)
        {
            const FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>();
            if (!block)
            {
                return;
            }

            return block->ArchetypeManager.ForEachComponent<T>(entityId, callback);
        }

        //
        //  Tag Management
        //

        // Gets a pointer to the tags array for a given world.
        static const decltype(FeatureECSDynamicBlock::Tags)* GetTags(WorldConstRef world);

        // Returns true if the entity has a given tag.
        static bool HasTag(WorldConstRef world, EntityId entityId, const FName& tag);

        // Adds a tag to the entity. Returns true if the tag was added.
        static bool AddTag(WorldRef world, EntityId entityId, const FName& tag);

        // Removes a tag from the entity. Returns true if the tag was removed.
        static bool RemoveTag(WorldRef world, EntityId entityId, const FName& tag);

        // Removes all tags from the entity. Returns the number of tags that were removed.
        static uint32 RemoveAllTags(WorldRef world, EntityId entityId);

        // Enumerates each tag of a given entity.
        template <class TCallback>
        static void ForEachTag(WorldConstRef world, EntityId entityId, const TCallback& callback)
        {
            if (const FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>())
            {
                block->Tags.ForEachTag(entityId, callback);
            }
        }

        //
        // Group management
        //

        // Gets a pointer to the groups array for a given world.
        static const decltype(FeatureECSDynamicBlock::Groups)* GetGroups(WorldConstRef world);

        // Returns true if the entity group contains an entity.
        static bool GroupContainsEntity(WorldConstRef world, EntityId group, EntityId entity);

        // Adds an entity to an entity group. Returns true if the entity was added.
        static bool AddEntityToGroup(WorldRef world, EntityId group, EntityId entity);

        // Removes an entity from an entity group. Returns true if the entity was removed.
        static bool RemoveEntityFromGroup(WorldRef world, EntityId group, EntityId entity);

        // Removes an entity from all groups that it belongs to. Returns the number of groups it was removed from.
        static uint32 RemoveEntityFromAllGroups(WorldRef world, EntityId entity);

        // Removes all entities from an entity group. Returns the number of entities that were removed.
        static uint32 ClearGroup(WorldRef world, EntityId group);

        // Returns the number of entities in a group.
        static uint32 GetGroupSize(WorldConstRef world, EntityId group);

        static EntityId GetFirstEntityInGroup(WorldConstRef world, EntityId group);

        static EntityId GetFirstEntityInGroup(WorldConstRef world, EntityId group, uint32& outIndex);
        
        static EntityId GetNextEntityInGroup(WorldConstRef world, EntityId group, uint32 currIndex, uint32& outIndex);

        template <class TCallback>
        static void ForEachEntityInGroup(WorldConstRef world, const EntityId group, const TCallback& callback)
        {
            if (const FeatureECSDynamicBlock* block = world.GetBlock<FeatureECSDynamicBlock>())
            {
                block->Groups.ForEachEntity(group, callback);
            }
        }

        //
        // Blackboard helpers
        //

        static Blackboard::blackboard_key_t CreateBlackboardKey(
            const EntityId& id,
            const FName& key,
            Blackboard::blackboard_type_t type = Blackboard::UnknownType);

        static bool HasBlackboardValue(
            WorldConstRef world,
            const EntityId& id,
            const FName& key,
            Blackboard::blackboard_type_t type = Blackboard::UnknownType);

        template <class T>
        static bool HasBlackboardValue(WorldConstRef world, const EntityId& id, const FName& key)
        {
            return HasBlackboardValue(world, id, key, Blackboard::BlackboardValueType<T>::Type);
        }

        static bool SetBlackboardValueRaw(
            WorldRef world,
            const EntityId& id,
            const FName& key,
            Blackboard::blackboard_value_t value,
            Blackboard::blackboard_type_t type = Blackboard::UnknownType);

        template <class T>
        static bool SetBlackboardValue(
            WorldRef world,
            const EntityId& id,
            const FName& key,
            const T& value)
        {
            Blackboard::FixedBlackboard& blackboard = Blackboard::FeatureBlackboard::GetBlackboard(world);
            Blackboard::blackboard_key_t fullKey = CreateBlackboardKey(id, key);
            return blackboard.SetValue<T>(fullKey, value);
        }

        static bool TryGetBlackboardValueRaw(
            WorldConstRef world,
            const EntityId& id,
            const FName& key,
            Blackboard::blackboard_value_t& outValue,
            Blackboard::blackboard_type_t expectedType = Blackboard::IgnoreType);

        template <class T>
        static bool TryGetBlackboardValue(
            WorldConstRef world,
            const EntityId& id,
            const FName& key,
            T& outValue)
        {
            const Blackboard::FixedBlackboard& blackboard = Blackboard::FeatureBlackboard::GetBlackboard(world);
            Blackboard::blackboard_key_t fullKey = CreateBlackboardKey(id, key);
            return blackboard.GetValue<T>(fullKey, outValue);
        }

        static Blackboard::blackboard_value_t GetBlackboardValueRaw(
            WorldConstRef world,
            const EntityId& id,
            const FName& key,
            Blackboard::blackboard_type_t expectedType = Blackboard::IgnoreType);

        template <class T>
        static T GetBlackboardValue(
            WorldConstRef world,
            const EntityId& id,
            const FName& key,
            const T& defaultValue = {})
        {
            const Blackboard::FixedBlackboard& blackboard = Blackboard::FeatureBlackboard::GetBlackboard(world);
            Blackboard::blackboard_key_t fullKey = CreateBlackboardKey(id, key);
            T result;
            if (!blackboard.GetValue<T>(fullKey, result))
            {
                result = defaultValue;
            }
            return result;
        }

        template <class T = Blackboard::blackboard_value_t>
        static bool RemoveBlackboardValue(
            WorldRef world,
            const EntityId& id,
            const FName& key,
            bool checkType = true)
        {
            Blackboard::FixedBlackboard& blackboard = Blackboard::FeatureBlackboard::GetBlackboard(world);
            Blackboard::blackboard_key_t fullKey = CreateBlackboardKey(id, key);
            return blackboard.RemoveValue<T>(fullKey, checkType);
        }

        //
        // Jobs
        //

        static CommandBuffer& GetCommandBuffer(WorldConstRef world);

        using TCommandHandler = std::function<void(WorldRef, const CommandBuffer::Command&)>;

        static void RegisterCommandHandler(WorldRef world, FName commandId, TCommandHandler handler);

        template <class TCommand>
        static void RegisterCommandHandler(WorldRef world, std::function<void(WorldRef, const TCommand&)> handler)
        {
            RegisterCommandHandler(world, TCommand::StaticId, [handler](WorldRef world, const CommandBuffer::Command& command)
            {
                handler(world, *static_cast<const TCommand*>(command.Data));
            });
        }

        // Register a job to run in the given phase. Returns a handle for dependency declarations.
        // Phase handles are scoped to their phase — AddJobDependency requires both handles from the same phase.
        static JobHandle RegisterJob(WorldRef world, std::unique_ptr<IJobBase> job, EJobPhase phase = EJobPhase::Update);
        static JobHandle RegisterJob(WorldRef world, IJobBase* job, EJobPhase phase = EJobPhase::Update);

        // Declare that 'after' must not start until 'before' completes within the same phase.
        static void AddJobDependency(WorldRef world, EJobPhase phase, JobHandle after, JobHandle before);

        // Returns the handle for the ECS PreUpdate sort job.
        // Valid after FeatureECS::OnWorldInitialize; use to declare downstream dependencies.
        static JobHandle GetPreUpdateSortJobHandle(WorldConstRef world);

        // Execute a caller-owned JobScheduler using this world's task queue and command buffers.
        // Rebuilds archetype batches if the archetype generation has changed since the last build.
        // Call from ISystem::OnXxxWorldUpdate to run system-owned job graphs with full parallelism.
        static void ExecuteScheduler(WorldRef world, JobScheduler& scheduler);

        // Read-only access to the global phase schedulers — for debug visualization only.
        static const JobScheduler& GetScheduler(WorldConstRef world, EJobPhase phase);

        // Register a named scheduler owned by a system for debug visualization.
        // The scheduler must outlive the world (system lifetime guarantees this).
        static void RegisterScheduler(WorldRef world, const JobScheduler& scheduler);

        // Returns all named schedulers registered via RegisterScheduler, in registration order.
        static std::vector<std::pair<FName, const JobScheduler*>> GetNamedSchedulers(WorldConstRef world);

        bool bAllowParallelJobs = true;

        //
        // Hierarchy/Transform
        //

        static const Transform2D* GetLocalTransformPtr(WorldConstRef world, EntityId entityId);
        static const Transform2D* GetWorldTransformPtr(WorldConstRef world, EntityId entityId);

        static Vec2 GetLocalPosition(WorldConstRef world, EntityId entityId);
        static Vec2 GetWorldPosition(WorldConstRef world, EntityId entityId);

        static Angle GetLocalFacing(WorldConstRef world, EntityId entityId);
        static Angle GetWorldFacing(WorldConstRef world, EntityId entityId);

        static Value GetLocalScale(WorldConstRef world, EntityId entityId);
        static Value GetWorldScale(WorldConstRef world, EntityId entityId);

        static EntityId GetParent(WorldConstRef world, EntityId entityId);

        //
        // Spacial Queries
        //

        // Returns true if the entity is within range of the target entity.
        static bool IsInRange(WorldConstRef world, EntityId entity, EntityId target, Distance range);

        // Returns true if the entity is within range of the target location.
        static bool IsInRange(WorldConstRef world, EntityId entity, const Vec2& target, Distance range);

        // Returns true if the entity is within range of the target entity.
        static bool IsFacing(WorldConstRef world, EntityId entity, EntityId target, Angle threshold);

        // Returns true if the entity is within range of the target location.
        static bool IsFacing(WorldConstRef world, EntityId entity, const Vec2& target, Angle threshold);

        static void QueryEntitiesInRange(
            WorldConstRef world,
            const Vec2& pos,
            Distance range,
            std::vector<EntityTransform>& outEntities,
            const EntityRangeQueryArgs& args = {});

        static void QueryEntitiesInRect(
            WorldConstRef world,
            const Vec2& min,
            const Vec2& max,
            std::vector<EntityTransform>& outEntities,
            const EntityRangeQueryArgs& args = {});

        bool bDebugDrawMortonCodeBoundaries = false;
        bool bDebugDrawEntityZCodes = false;

    private:

        void Initialize(const std::shared_ptr<Phoenix::Session>& session) override;

        void OnPreUpdate(const FeatureUpdateArgs& args) override;
        void OnUpdate(const FeatureUpdateArgs& args) override;
        void OnPostUpdate(const FeatureUpdateArgs& args) override;

        bool OnPreHandleAction(const FeatureActionArgs& action) override;
        bool OnHandleAction(const FeatureActionArgs& action) override;
        bool OnPostHandleAction(const FeatureActionArgs& action) override;

        void OnWorldLayout(const WorldLayoutContext& context, BlockBufferConfigBuilder& builder) override;
        void OnWorldInitialize(WorldRef world) override;
        void OnWorldShutdown(WorldRef world) override;

        void OnPreWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;
        void OnWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;
        void OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;

        bool OnPreHandleWorldAction(WorldRef world, const FeatureActionArgs& action) override;
        bool OnHandleWorldAction(WorldRef world, const FeatureActionArgs& action) override;
        bool OnPostHandleWorldAction(WorldRef world, const FeatureActionArgs& action) override;

        void OnDebugRender(WorldConstRef world, const IDebugState& state, IDebugRenderer& renderer) override;

        static void SortAndCompact(WorldRef world);

        void OnReclaimEntity(WorldRef world, const EntityId& entityId) const;

        void ApplyCommandBuffers(WorldRef world);

        JobScheduler& GetMutableScheduler(WorldConstRef world, EJobPhase phase);
        void BuildAllSchedulers(WorldRef world, const ArchetypeManager& archetypes);
        void RebuildAllSchedulersIfDirty(WorldRef world, const ArchetypeManager& archetypes);
        void ExecuteScheduler(WorldRef world, EJobPhase phase);

        void RegisterECSCommandHandlers();

        std::vector<std::shared_ptr<ISystem>> Systems;

        std::vector<std::unique_ptr<CommandBuffer>> CommandBuffers;
        std::unordered_map<FName, TCommandHandler> CommandHandlers;

        struct ScopedWorldData
        {
            std::unique_ptr<JobScheduler> PreUpdateScheduler;
            std::unique_ptr<JobScheduler> UpdateScheduler;
            std::unique_ptr<JobScheduler> PostUpdateScheduler;
            JobHandle PreUpdateSortJobHandle = InvalidJobHandle;

            // Named schedulers registered by systems for debug visualization.
            std::vector<std::pair<FName, const JobScheduler*>> NamedSchedulers;
        };

        ScopedWorldData& GetScopedWorldData(WorldConstRef world);

        std::unordered_map<FName, ScopedWorldData> WorldData;

        // Jobs registered directly (as opposed to owned by systems) that need to be kept alive.
        std::vector<std::unique_ptr<IJobBase>> OwnedJobs;

        FOnEntityAcquired EntityAcquiredEvent;
        FOnEntityReleasing EntityReleasedEvent;
        FOnEntityReleased EntityDestroyedEvent;
    };
}

PHX_DEFINE_TYPE(Phoenix::ECS::FeatureECS)
{
    registration
        .Namespace("Phoenix.ECS")
        .Field("bDebugDrawMortonCodeBoundaries",                                &ECS::FeatureECS::bDebugDrawMortonCodeBoundaries)
        .Field("bDebugDrawEntityZCodes",                                        &ECS::FeatureECS::bDebugDrawEntityZCodes)
        // Entity Management
        .StaticMethod("AcquireEntity(world, kind)",                         &ECS::FeatureECS::StaticAcquireEntity)
        .StaticMethod("ReleaseEntity(world, entity)",                       &ECS::FeatureECS::StaticReleaseEntity)
        .StaticMethod("SetEntityKind(world, entity, kind)",                 &ECS::FeatureECS::SetEntityKind)
        // Tags
        .StaticMethod("HasTag(world, entity, tag)",                         &ECS::FeatureECS::HasTag)
        .StaticMethod("AddTag(world, entity, tag)",                         &ECS::FeatureECS::AddTag)
        .StaticMethod("RemoveTag(world, entity, tag)",                      &ECS::FeatureECS::RemoveTag)
        .StaticMethod("RemoveAllTags(world, entity)",                       &ECS::FeatureECS::RemoveAllTags)
        // Groups
        .StaticMethod("GroupContainsEntity(world, group, entity)",          &ECS::FeatureECS::GroupContainsEntity)
        .StaticMethod("AddEntityToGroup(world, group, entity)",             &ECS::FeatureECS::AddEntityToGroup)
        .StaticMethod("RemoveEntityFromGroup(world, group, entity)",        &ECS::FeatureECS::RemoveEntityFromGroup)
        .StaticMethod("RemoveEntityFromAllGroups(world, entity)",           &ECS::FeatureECS::RemoveEntityFromAllGroups)
        .StaticMethod("ClearGroup(world, group)",                           &ECS::FeatureECS::ClearGroup)
        .StaticMethod("GetGroupSize(world, group)",                         &ECS::FeatureECS::GetGroupSize)
        // Blackboard
        .StaticMethod("CreateBlackboardKey(id, key, type)",                 &ECS::FeatureECS::CreateBlackboardKey)
        .StaticMethod("HasBlackboardValue(world, id, key, type)",           StaticMethodCast<bool, WorldConstRef, const ECS::EntityId&, const FName&, Blackboard::blackboard_type_t>(&ECS::FeatureECS::HasBlackboardValue))
        .StaticMethod("SetBlackboardValue(world, id, key, value)",          &ECS::FeatureECS::SetBlackboardValue<Distance>)
        .StaticMethod("GetBlackboardValue(world, id, key, defaultValue)",   &ECS::FeatureECS::GetBlackboardValue<Distance>)
        .StaticMethod("RemoveBlackboardValue(world, id, key, bool)",        &ECS::FeatureECS::RemoveBlackboardValue<Distance>)
        // Transforms
        .StaticMethod("GetLocalPosition(world, entity)",                    &ECS::FeatureECS::GetLocalPosition)
        .StaticMethod("GetLocalFacing(world, entity)",                      &ECS::FeatureECS::GetLocalFacing)
        .StaticMethod("GetLocalScale(world, entity)",                       &ECS::FeatureECS::GetLocalScale)
        .StaticMethod("GetWorldPosition(world, entity)",                    &ECS::FeatureECS::GetWorldPosition)
        .StaticMethod("GetWorldFacing(world, entity)",                      &ECS::FeatureECS::GetWorldFacing)
        .StaticMethod("GetWorldScale(world, entity)",                       &ECS::FeatureECS::GetWorldScale)
        .StaticMethod("GetParent(world, entity)",                           &ECS::FeatureECS::GetParent)
        // Utility
        .StaticMethod("IsInRangeOfEntity(world, entity, target, range)",    StaticMethodCast<bool, WorldConstRef, ECS::EntityId, ECS::EntityId, Distance>(&ECS::FeatureECS::IsInRange))
        .StaticMethod("IsInRangeOfPos(world, entity, target, range)",       StaticMethodCast<bool, WorldConstRef, ECS::EntityId, const Vec2&, Distance>(&ECS::FeatureECS::IsInRange))
        .StaticMethod("IsFacingEntity(world, entity, target, threshold)",   StaticMethodCast<bool, WorldConstRef, ECS::EntityId, ECS::EntityId, Angle>(&ECS::FeatureECS::IsFacing))
        .StaticMethod("IsFacingPos(world, entity, target, threshold)",      StaticMethodCast<bool, WorldConstRef, ECS::EntityId, const Vec2&, Angle>(&ECS::FeatureECS::IsFacing))
    ;
}