
#pragma once

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Name.h"
#include "PhoenixSim/Profiling.h"
#include "PhoenixSim/WorldsFwd.h"
#include "PhoenixSim/Containers/FixedMap.h"
#include "PhoenixSim/Containers/FixedBlockAllocator.h"
#include "PhoenixSim/ECS/ArchetypeDefinition.h"
#include "PhoenixSim/ECS/ArchetypeList.h"
#include "PhoenixSim/ECS/EntityQuery.h"

namespace Phoenix
{
    namespace ECS
    {
        template <class ...TComponents>
        using TEntityQueryFunc = std::function<void(EntityId, TComponents...)>;

        template <class ...TComponents>
        using TEntityQueryBufferFunc = std::function<void(const EntityComponentSpan<TComponents...>&)>;

        template <class TJob>
        struct EntityJobHelper
        {
            template <class Fn>
            struct ExecuteFnTraits;

            template <class ...TComponents>
            struct ExecuteFnTraits<void (TJob::*)(WorldRef, const EntityComponentSpan<TComponents...>&)>
            {
                using TEntityComponentSpan = EntityComponentSpan<TComponents...>;

                static EntityQuery BuildQuery()
                {
                    EntityQueryBuilder builder;
                    builder.RequireAllComponents<TComponents...>();
                    return builder.GetQuery();
                }
            };

            template <class ...TComponents>
            struct ExecuteFnTraits<void (TJob::*)(WorldRef, const EntityComponentSpan<TComponents...>&) const>
            {
                using TEntityComponentSpan = EntityComponentSpan<TComponents...>;

                static EntityQuery BuildQuery()
                {
                    EntityQueryBuilder builder;
                    builder.RequireAllComponents<TComponents...>();
                    return builder.GetQuery();
                }
            };

            using THelper = ExecuteFnTraits<decltype(&TJob::Execute)>;
            using TEntityComponentSpan = THelper::TEntityComponentSpan;

            static EntityQuery BuildQuery()
            {
                return THelper::BuildQuery();
            }
        };

        class PHOENIX_SIM_API ArchetypeManager
        {
        public:

            using TArchetypeList = FixedArchetypeList;
            using TArchetypeListAllocator = FixedBlockAllocator;
            using TBlockHandle = TArchetypeListAllocator::Handle;
            using TArchetypeHandle = TArchetypeList::Handle;
            using TComponentDefMap = TFixedMap<FName, ComponentDefinition>;
            using TArchetypeDefMap = TFixedMap<FName, ArchetypeDefinition>;
            using TEntityHandleMap = TFixedMap<EntityId, TArchetypeHandle>;

            struct Config
            {
                uint32 MaxComponentDefs = 0;
                uint32 MaxArchetypeDefs = 0;
                uint32 MaxArchetypeLists = 0;
                uint32 ArchetypeListSize = 0;
                uint32 MaxEntities = 0;
            };

            ArchetypeManager() = default;

            template <class TAllocator>
            ArchetypeManager(TAllocator& allocator, const Config& config)
                : ComponentDefinitions(allocator, config.MaxComponentDefs)
                , ArchetypeDefinitions(allocator, config.MaxArchetypeDefs)
                , ArchetypeLists(allocator, { static_cast<uint32>(config.ArchetypeListSize + sizeof(TArchetypeList)), config.MaxArchetypeLists })
                , EntityHandles(allocator, config.MaxEntities)
                , Configuration(config)
            {
            }

            template <class TAllocator>
            ArchetypeManager(TAllocator& allocator, const Config& config, const ArchetypeManager& other)
                : ComponentDefinitions(allocator, config.MaxComponentDefs, other.ComponentDefinitions)
                , ArchetypeDefinitions(allocator, config.MaxArchetypeDefs, other.ArchetypeDefinitions)
                , ArchetypeLists(allocator, { static_cast<uint32>(config.ArchetypeListSize + sizeof(TArchetypeList)), config.MaxArchetypeLists }, other.ArchetypeLists)
                , EntityHandles(allocator, config.MaxEntities, other.EntityHandles)
                , Configuration(config)
            {
            }

            static uint32 GetAllocSizeBytes(const Config& config);

            uint32 GetAllocSizeBytes() const;

            bool IsValid(TArchetypeHandle handle) const;

            uint32 GetNumActiveArchetypes() const;

            uint32 GetNumArchetypeLists() const;

            //
            // Archetype Definitions
            //

            const TArchetypeDefMap& GetArchetypeDefinitions() const;

            bool RegisterArchetypeDefinition(const ArchetypeDefinition& definition);

            bool UnregisterArchetypeDefinition(const ArchetypeDefinition& definition);

            bool IsArchetypeRegistered(const FName& archetypeIdOrHash) const;

            FName GetArchetypeHash(const FName& archetypeId) const;

            const ArchetypeDefinition* GetArchetypeDefinitionByHash(const FName& archetypeHash) const;

            const ArchetypeDefinition* GetArchetypeDefinitionById(const FName& archetypeId) const;

            const ArchetypeDefinition* GetArchetypeDefinition(const FName& archetypeIdOrHash) const;

            //
            // Component Definitions
            //

            const TComponentDefMap& GetComponentDefinitions() const;

            bool RegisterComponentDefinition(const ComponentDefinition& definition);

            bool UnregisterComponentDefinition(const FName& componentId);

            bool IsComponentRegistered(const FName& componentId) const;

            // Acquire an archetype for a given entity
            TArchetypeHandle Acquire(EntityId entityId, const FName& archetypeIdOrHash);

            // Release an archetype back to the pool
            bool Release(const TArchetypeHandle& handle);

            // Release an archetype back to the pool
            bool Release(EntityId entityId);

            // Sets the archetype of a given entity to a new archetype.
            // Data for any components shared between the current archetype and the new archetype are preserved.
            TArchetypeHandle SetArchetype(TArchetypeHandle& inOutHandle, const FName& archetypeIdOrHash);

            // Sets the archetype of a given entity to a new archetype.
            // Data for any components shared between the current archetype and the new archetype are preserved.
            TArchetypeHandle SetArchetype(EntityId entityId, const FName& archetypeIdOrHash);

            void* AddComponent(
                TArchetypeHandle& inOutHandle,
                const ComponentDefinition& componentDef,
                const void* componentData = nullptr);

            void* AddComponent(
                EntityId entityId,
                const ComponentDefinition& componentDef,
                const void* componentData = nullptr);

            void* AddComponent(
                TArchetypeHandle& inOutHandle,
                const FName& componentId,
                const void* componentData = nullptr);

            void* AddComponent(
                EntityId entityId,
                const FName& componentId,
                const void* componentData = nullptr);

            template <class T>
            T* AddComponent(TArchetypeHandle& inOutHandle, const T& defaultValue = {})
            {
                ComponentDefinition compDef = ComponentDefinition::Create<T>();
                return static_cast<T*>(AddComponent(inOutHandle, compDef, &defaultValue));
            }

            template <class T>
            T* AddComponent(EntityId entityId, const T& defaultValue = {})
            {
                TArchetypeHandle handle = GetHandleForEntity(entityId);
                return AddComponent<T>(handle, defaultValue);
            }

            template <class T, class ...TArgs>
            T* EmplaceComponent(TArchetypeHandle& inOutHandle, const TArgs&...args)
            {
                ComponentDefinition compDef = ComponentDefinition::Create<T>();
                T* compPtr = static_cast<T*>(AddComponent(inOutHandle, compDef));
                if (!compPtr)
                {
                    return nullptr;
                }
                new (compPtr) T(args...);
                return compPtr;
            }

            template <class T, class ...TArgs>
            T* EmplaceComponent(EntityId entityId, const TArgs&...args)
            {
                TArchetypeHandle handle = GetHandleForEntity(entityId);
                return EmplaceComponent<T, TArgs...>(handle, std::forward<TArgs>(args)...);
            }

            bool RemoveComponent(TArchetypeHandle& inOutHandle, const FName& componentId);
            bool RemoveComponent(EntityId entityId, const FName& componentId);

            template <class T>
            bool RemoveComponent(const TArchetypeHandle& handle)
            {
                return RemoveComponent(handle, StaticTypeName<T>::TypeId);
            }

            template <class T>
            bool RemoveComponent(EntityId entityId)
            {
                return RemoveComponent(entityId, StaticTypeName<T>::TypeId);
            }

            bool RemoveAllComponents(const TArchetypeHandle& handle);
            bool RemoveAllComponents(EntityId entityId);

            void* GetComponent(const TArchetypeHandle& handle, const FName& componentId);
            const void* GetComponent(const TArchetypeHandle& handle, const FName& componentId) const;

            void* GetComponent(EntityId entityId, const FName& componentId);
            const void* GetComponent(EntityId entityId, const FName& componentId) const;

            template <class T>
            T* GetComponent(const TArchetypeHandle& handle)
            {
                PHX_PROFILE_ZONE_SCOPED;

                TArchetypeList* list = FindOwningArchetypeList(handle);
                if (!list || !list->IsValid(handle))
                {
                    return nullptr;
                }

                return list->GetComponent<T>(handle);
            }

            template <class T>
            const T* GetComponent(const TArchetypeHandle& handle) const
            {
                PHX_PROFILE_ZONE_SCOPED;

                const TArchetypeList* list = FindOwningArchetypeList(handle);
                if (!list || !list->IsValid(handle))
                {
                    return nullptr;
                }

                return list->GetComponent<T>(handle);
            }

            template <class T>
            T* GetComponent(EntityId entityId)
            {
                TArchetypeHandle handle = GetHandleForEntity(entityId);
                return GetComponent<T>(handle);
            }

            template <class T>
            const T* GetComponent(EntityId entityId) const
            {
                TArchetypeHandle handle = GetHandleForEntity(entityId);
                return GetComponent<T>(handle);
            }

            TArchetypeList* FindFirstArchetypeList(const FName& archetypeIdOrHash, bool includeFullLists = false);

            TArchetypeList* FindOwningArchetypeList(const TArchetypeHandle& handle);
            const TArchetypeList* FindOwningArchetypeList(const TArchetypeHandle& handle) const;

            TArchetypeList* FindOwningArchetypeList(EntityId entityId);
            const TArchetypeList* FindOwningArchetypeList(EntityId entityId) const;

            void ForEachArchetypeList(const std::function<void(TArchetypeList&)>& func);

            void ForEachArchetypeList(const std::function<void(const TArchetypeList&)>& func) const;

            void ForEachArchetypeList(const FName& archetypeIdOrHash, const std::function<void(TArchetypeList&)>& func);

            template <class ...TComponents>
            void ForEachEntity(const EntityQuery& query, const TEntityQueryFunc<TComponents...>& func)
            {
                for (TBlockHandle handle : ArchetypeLists)
                {
                    TArchetypeList* list = ArchetypeLists.GetPtr<TArchetypeList>(handle);
                    if (list && query.PassesFilter(list->GetDefinition()))
                    {
                        list->ForEachEntity<TComponents...>([&](EntityId entityId, TComponents ...components)
                        {
                            func(entityId, std::forward<TComponents>(components)...);
                        });
                    }
                }
            }

            template <class ...TComponents>
            void ForEachEntity(const EntityQuery& query, const TEntityQueryFunc<TComponents...>& func) const
            {
                for (TBlockHandle handle : ArchetypeLists)
                {
                    const TArchetypeList* list = ArchetypeLists.GetPtr<TArchetypeList>(handle);
                    if (list && query.PassesFilter(list->GetDefinition()))
                    {
                        list->ForEachEntity<TComponents...>([&](EntityId entityId, TComponents ...components)
                        {
                            func(entityId, std::forward<TComponents>(components)...);
                        });
                    }
                }
            }

            template <class ...TComponents>
            void ForEachEntity(const EntityQuery& query, const TEntityQueryBufferFunc<TComponents...>& func)
            {
                uint32 startingIndex = 0;
                for (TBlockHandle handle : ArchetypeLists)
                {
                    TArchetypeList* list = ArchetypeLists.GetPtr<TArchetypeList>(handle);
                    if (list && query.PassesFilter(list->GetDefinition()))
                    {
                        func(EntityComponentSpan<TComponents...>::FromList(*list, startingIndex));
                        startingIndex += list->GetNumInstances();
                    }
                }
            }

            template <class ...TComponents>
            void ForEachEntity(const EntityQuery& query, const TEntityQueryBufferFunc<TComponents...>& func) const
            {
                uint32 startingIndex = 0;
                for (TBlockHandle handle : ArchetypeLists)
                {
                    const TArchetypeList* list = ArchetypeLists.GetPtr<TArchetypeList>(handle);
                    if (list && query.PassesFilter(list->GetDefinition()))
                    {
                        func(EntityComponentSpan<TComponents...>::FromList(*list, startingIndex));
                        startingIndex += list->GetNumInstances();
                    }
                }
            }

            template <class TJob>
            void ForEachEntity(WorldRef world, const EntityQuery& query, TJob& job)
            {
                if constexpr ( requires { job.Begin(world); })
                {
                    job.Begin(world);
                }
                uint32 startingIndex = 0;
                for (TBlockHandle handle : ArchetypeLists)
                {
                    TArchetypeList* list = ArchetypeLists.GetPtr<TArchetypeList>(handle);
                    if (list && query.PassesFilter(list->GetDefinition()))
                    {
                        job.Execute(world, EntityJobHelper<TJob>::TEntityComponentSpan::FromList(*list, startingIndex));
                        startingIndex += list->GetNumInstances();
                    }
                }
                if constexpr ( requires { job.End(world); })
                {
                    job.End(world);
                }
            }

            template <class TJob>
            void ForEachEntity(WorldConstRef world, const EntityQuery& query, TJob& job)
            {
                if constexpr ( requires { job.Begin(world); })
                {
                    job.Begin(world);
                }
                uint32 startingIndex = 0;
                for (TBlockHandle handle : ArchetypeLists)
                {
                    TArchetypeList* list = ArchetypeLists.GetPtr<TArchetypeList>(handle);
                    if (list && query.PassesFilter(list->GetDefinition()))
                    {
                        job.Execute(world, EntityJobHelper<TJob>::TEntityComponentSpan::FromList(*list, startingIndex));
                        startingIndex += list->GetNumInstances();
                    }
                }
                if constexpr ( requires { job.End(world); })
                {
                    job.End(world);
                }
            }

            template <class T>
            void ForEachComponent(EntityId entityId, const T& callback)
            {
                TArchetypeList* list = FindOwningArchetypeList(entityId);
                if (!list)
                {
                    return;
                }

                ArchetypeHandle handle = GetHandleForEntity(entityId);
                list->ForEachComponent(handle, callback);
            }

            template <class T>
            void ForEachComponent(EntityId entityId, const T& callback) const
            {
                ArchetypeHandle handle = GetHandleForEntity(entityId);
                const TArchetypeList* list = FindOwningArchetypeList(entityId);
                if (!list)
                {
                    return;
                }

                list->ForEachComponent(handle, callback);
            }

            void Compact();

            uint32 GetGeneration() const;

        private:

            TArchetypeHandle GetHandleForEntity(EntityId entityId) const;

            TArchetypeList* FindOrAddArchetypeList(const FName& archetypeIdOrHash);

            // A mapping of component id to component definition.
            TComponentDefMap ComponentDefinitions;

            // A mapping of archetype hash to archetype definition.
            TArchetypeDefMap ArchetypeDefinitions;

            TArchetypeListAllocator ArchetypeLists;

            TEntityHandleMap EntityHandles;

            Config Configuration;

            uint32 Generation = 0;
        };
    }
}
