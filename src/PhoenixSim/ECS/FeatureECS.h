
#pragma once

#include "PhoenixSim/Delegates.h"
#include "PhoenixSim/Features.h"
#include "PhoenixSim/Parallel.h"
#include "PhoenixSim/Worlds.h"
#include "PhoenixSim/Blackboard/FeatureBlackboard.h"
#include "PhoenixSim/Blackboard/FixedBlackboard.h"
#include "PhoenixSim/ECS/ArchetypeManager.h"
#include "PhoenixSim/ECS/Entity.h"
#include "PhoenixSim/ECS/EntityId.h"
#include "PhoenixSim/ECS/FixedEntityList.h"
#include "PhoenixSim/ECS/FixedGroupList.h"
#include "PhoenixSim/ECS/FixedTagList.h"
#include "PhoenixSim/ECS/SystemJob.h"
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

    struct PHOENIX_SIM_API FeatureECSDynamicBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_WITH_ALLOC(FeatureECSDynamicBlock)

        struct Config
        {
            uint32 MaxEntities = 0;
            uint32 MaxTags = 0;
            uint32 MaxGroups = 0;
            ArchetypeManager::Config ArchetypeManager;
        };

        FixedEntityList Entities;
        FixedTagList Tags;
        FixedGroupList Groups;
        ArchetypeManager ArchetypeManager;
    };

    struct PHOENIX_SIM_API FeatureECSScratchBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_WITH_ALLOC(FeatureECSScratchBlock)

        struct Config
        {
            uint32 MaxEntities = 0;
        };

        TFixedArray<EntityTransform> SortedEntities;
        TAtomic<uint32> SortedEntityCount = 0;
    };

    struct PHOENIX_SIM_API FeatureECSCtorArgs
    {
        TVector<TSharedPtr<ISystem>> Systems;
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
        PHX_DECLARE_FEATURE_TYPE_BEGIN(FeatureECS)
            PHX_REGISTER_FIELD(bool, bDebugDrawMortonCodeBoundaries)
            PHX_REGISTER_FIELD(bool, bDebugDrawEntityZCodes)
        PHX_DECLARE_FEATURE_TYPE_END()

    public:

        FeatureECS();
        FeatureECS(const FeatureECSCtorArgs& args);

        void OnPreUpdate(const FeatureUpdateArgs& args) override;
        void OnUpdate(const FeatureUpdateArgs& args) override;
        void OnPostUpdate(const FeatureUpdateArgs& args) override;

        bool OnPreHandleAction(const FeatureActionArgs& action) override;
        bool OnHandleAction(const FeatureActionArgs& action) override;
        bool OnPostHandleAction(const FeatureActionArgs& action) override;

        void OnWorldLayout(const WorldLayoutContext& context, WorldLayoutBuilder& builder) override;
        void OnWorldInitialize(WorldRef world) override;
        void OnWorldShutdown(WorldRef world) override;

        bool InitView(WorldConstRef world, ViewContext& context) override;
        void FillView(WorldConstRef world, const ViewContext& context) override;

        void OnPreWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;
        void OnWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;
        void OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;

        bool OnPreHandleWorldAction(WorldRef world, const FeatureActionArgs& action) override;
        bool OnHandleWorldAction(WorldRef world, const FeatureActionArgs& action) override;
        bool OnPostHandleWorldAction(WorldRef world, const FeatureActionArgs& action) override;

        void OnDebugRender(WorldConstRef world, const IDebugState& state, IDebugRenderer& renderer) override;

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
        void RegisterSystem(const TSharedPtr<ISystem>& system);

        // Unregisters an existing ECS system. Returns true if the system was removed.
        bool UnregisterSystem(const TSharedPtr<ISystem>& system);

        const TVector<TSharedPtr<ISystem>>& GetSystems() const;

        //
        // Entity Management
        //

        // Gets a pointer to the entities array for a given world.
        static const decltype(FeatureECSDynamicBlock::Entities)* GetEntities(WorldConstRef world);
        
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
            return UnregisterComponentDefinition(world, TComponent::StaticTypeName);
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
            IComponent* comp = GetComponent(world, entityId, T::StaticTypeName);
            return static_cast<T*>(comp);
        }

        // Gets the pointer to a component on an entity if it exists.
        template <class T>
        static const T* GetComponent(WorldConstRef world, EntityId entityId)
        {
            const IComponent* comp = GetComponent(world, entityId, T::StaticTypeName);
            return static_cast<const T*>(comp);
        }

        // Gets the pointer to a component on an entity or adds it if it doesn't exist.
        template <class T>
        static T* GetOrAddComponent(WorldRef world, EntityId entityId)
        {
            IComponent* comp = GetComponent(world, entityId, T::StaticTypeName);
            if (!comp)
            {
                comp = AddComponent<T>(world, entityId);
            }
            return static_cast<T*>(comp);
        }

        // Gets a reference to a component on an entity if it exists.
        template <class T>
        static T& GetComponentRef(WorldRef world, EntityId entityId)
        {
            IComponent& comp = GetComponentRef(world, entityId, T::StaticTypeName);
            return static_cast<T&>(comp);
        }

        // Gets a reference to a component on an entity if it exists.
        template <class T>
        static const T& GetComponentRef(WorldConstRef world, EntityId entityId)
        {
            const IComponent& comp = GetComponentRef(world, entityId, T::StaticTypeName);
            return static_cast<const T&>(comp);
        }

        // Returns whether an entity is using an archetype containing a type of component.
        static bool HasComponent(WorldConstRef world, EntityId entityId, const FName& componentType);

        // Adds a new component to an entity.
        static IComponent* AddComponent(WorldRef world, EntityId entityId, const FName& componentType);

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
            return RemoveComponent(world, entityId, TComponent::StaticTypeName);
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

        template <class TJob>
        static void Schedule(WorldRef world, const TJob& job)
        {
            TSharedPtr<TaskQueue> taskQueue = TaskQueue::GetTaskQueue((uint32)world.GetId());

            FeatureECSDynamicBlock& dynamicBlock = world.GetBlockRef<FeatureECSDynamicBlock>();
            WorldPtr worldPtr = &world;

            uint32 startIndex = 0;
            dynamicBlock.ArchetypeManager.ForEachArchetypeList([&](FixedArchetypeList& list)
            {
                if (job.GetQuery().PassesFilter(list.GetDefinition()))
                {
                    TJob jobInstance = job;
                    auto listPtr = &list;
                    auto wrapper = [=]() mutable
                    {
                        static_cast<IEntityJobBase*>(&jobInstance)->Execute(*worldPtr, *listPtr, startIndex);
                    };

                    taskQueue->Enqueue(std::move(wrapper));

                    startIndex += list.GetNumInstances();
                }
            });
        }

        template <class TJob>
        static void ScheduleParallel(WorldRef world, const TJob& job)
        {
            PHX_PROFILE_ZONE_SCOPED;

            TSharedPtr<TaskQueue> taskQueue = TaskQueue::GetTaskQueue((uint32)world.GetId());

            FeatureECSDynamicBlock& dynamicBlock = world.GetBlockRef<FeatureECSDynamicBlock>();
            WorldPtr worldPtr = &world;

            uint32 numArchetypeLists = dynamicBlock.ArchetypeManager.GetNumArchetypeLists();
            std::vector<Task>& taskGroup = taskQueue->BeginGroup(numArchetypeLists);

            uint32 startIndex = 0;
            dynamicBlock.ArchetypeManager.ForEachArchetypeList([&](FixedArchetypeList& list)
            {
                if (job.GetQuery().PassesFilter(list.GetDefinition()))
                {
                    PHX_PROFILE_ZONE_SCOPED_N("PushTaskToTaskGroup");

                    TJob jobInstance = job;
                    auto listPtr = &list;
                    auto wrapper = [=]() mutable
                    {
                        static_cast<IEntityJobBase*>(&jobInstance)->Execute(*worldPtr, *listPtr, startIndex);
                    };

                    taskGroup.emplace_back(std::move(wrapper));

                    startIndex += list.GetNumInstances();
                }
            });

            taskQueue->EndGroup();
        }

        static const Transform2D* GetLocalTransformPtr(WorldConstRef world, EntityId entityId);
        
        static const Transform2D* GetWorldTransformPtr(WorldConstRef world, EntityId entityId);

        static Vec2 GetWorldPosition(WorldConstRef world, EntityId entityId);

        static Angle GetWorldFacing(WorldConstRef world, EntityId entityId);

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
            TVector<EntityTransform>& outEntities,
            const EntityRangeQueryArgs& args = {});

        static void QueryEntitiesInRect(
            WorldConstRef world,
            const Vec2& min,
            const Vec2& max,
            TVector<EntityTransform>& outEntities,
            const EntityRangeQueryArgs& args = {});

        bool bDebugDrawMortonCodeBoundaries = false;
        bool bDebugDrawEntityZCodes = false;

    private:

        static void SortEntitiesByZCode(WorldRef world);

        static void SortAndCompact(WorldRef world);

        void OnReclaimEntity(WorldRef world, const EntityId& entityId) const;

        TVector<TSharedPtr<ISystem>> Systems;
        TSharedPtr<ThreadPool> JobThreadPool;

        FOnEntityAcquired EntityAcquiredEvent;
        FOnEntityReleasing EntityReleasedEvent;
        FOnEntityReleased EntityDestroyedEvent;
    };
}
