#pragma once

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Name.h"
#include "PhoenixSim/ECS/ArchetypeHandle.h"
#include "PhoenixSim/ECS/ArchetypeDefinition.h"
#include "PhoenixSim/ECS/EntityId.h"

namespace Phoenix
{
    namespace ECS
    {
        template <class>
        struct ComponentAccessor
        {
        };

        template <class TComp>
        struct ComponentAccessor<const TComp&>
        {
            using TCompUnderlying = Underlying_T<TComp>;
            static const TComp& GetComponentRef(TCompUnderlying* data)
            {
                return *data;
            }
        };

        template <class TComp>
        struct ComponentAccessor<TComp&>
        {
            using TCompUnderlying = Underlying_T<TComp>;
            static TComp& GetComponentRef(TCompUnderlying* data)
            {
                return *data;
            }
        };

        struct ArchetypeInstance
        {
            EntityId EntityId;
        };

        // Archetype data is tightly packed into the Data buffer.
        // Data: [Entity0][Comp0][Comp1][Entity1][Comp0][Comp1]...
        class PHOENIX_SIM_API FixedArchetypeList
        {
        public:

            using Handle = ArchetypeHandle;

            FixedArchetypeList() = default;

            template <class TAllocator>
            FixedArchetypeList(TAllocator& allocator, uint32 capacity, const ArchetypeDefinition& definition, uint32 id = 0)
                : Capacity(capacity)
                , Id(id)
                , Definition(definition)
                , Data(allocator, capacity)
            {
            }

            template <class TAllocator>
            FixedArchetypeList(TAllocator& allocator, uint32 capacity, const FixedArchetypeList& other)
                : Capacity(capacity)
                , Id(other.Id)
                , Definition(other.Definition)
                , Data(allocator, capacity, other.Data)
            {
            }

            static uint32 GetAllocSizeBytes(uint32 capacity);

            uint32 GetAllocSizeBytes() const;

            uint32 GetCapacity() const;

            uint32 GetId() const;

            void SetId(uint32 id);

            const ArchetypeDefinition& GetDefinition() const;

            bool HasArchetypeDefinition(const FName& archetypeIdOrHash) const;

            void* GetData();

            const void* GetData() const;

            uint32 GetSize() const;

            bool IsFull() const;

            uint32 GetNumInstances() const;

            uint32 GetNumActiveInstances() const;

            uint32 GetInstanceCapacity() const;

            bool IsValid(const Handle& handle) const;

            bool OwnsHandle(const Handle& handle) const;

            Handle Acquire(EntityId entityId);

            bool Release(const Handle& handle);

            void* GetComponent(const Handle& handle, const FName& componentId);

            const void* GetComponent(const Handle& handle, const FName& componentId) const;

            template <class T>
            T* GetComponent(const Handle& handle)
            {
                void* dataPtr = GetComponent(handle, T::StaticTypeName);
                return static_cast<T*>(dataPtr);
            }

            template <class T>
            const T* GetComponent(const Handle& handle) const
            {
                const void* dataPtr = GetComponent(handle, T::StaticTypeName);
                return static_cast<const T*>(dataPtr);
            }

            // Gets the total size of an entity in the Data buffer.
            uint32 GetEntityTotalSize() const;

            // Gets the local offset of a component within an entity in the list.
            // Returns -1 if a component with the given id could not be found.
            uint32 GetComponentLocalOffset(const FName& componentId) const;

            void ForEachInstance(const std::function<void(const Handle&)>& func) const;

            template <class ...TComponents>
            void ForEachEntity(const std::function<void(EntityId, TComponents...)>& func)
            {
                for (uint32 i = 0; i < NumInstances; ++i)
                {
                    ArchetypeInstance* instance = GetEntityPtr(i);

                    if (instance->EntityId == EntityId::Invalid)
                    {
                        continue;
                    }

                    if (InvokeForEachCallbackWithIndex(func, i, instance->EntityId, GetComponentRef<TComponents>(i)...))
                    {
                        break;
                    }
                }
            }

            template <class ...TComponents>
            void ForEachEntity(const std::function<void(EntityId, TComponents...)>& func) const
            {
                for (uint32 i = 0; i < NumInstances; ++i)
                {
                    const ArchetypeInstance* instance = GetEntityPtr(i);

                    if (instance->EntityId == EntityId::Invalid)
                    {
                        continue;
                    }

                    if (InvokeForEachCallbackWithIndex(func, i, instance->EntityId, GetComponentRef<TComponents>(i)...))
                    {
                        break;
                    }
                }
            }

            template <class TCallback>
            void ForEachComponent(const Handle& handle, const TCallback& func)
            {
                for (uint8 i = 0; i < Definition.GetNumComponents(); ++i)
                {
                    const ComponentDefinition& componentDefinition = Definition[i];
                    void* compPtr = GetComponent(handle, componentDefinition.Id);
                    if (compPtr && InvokeForEachCallbackWithIndex(func, i, componentDefinition, compPtr))
                    {
                        break;
                    }
                }
            }

            template <class TCallback>
            void ForEachComponent(const Handle& handle, const TCallback& func) const
            {
                for (uint8 i = 0; i < Definition.GetNumComponents(); ++i)
                {
                    const ComponentDefinition& componentDefinition = Definition[i];
                    const void* compPtr = GetComponent(handle, componentDefinition.Id);
                    if (compPtr && InvokeForEachCallbackWithIndex(func, i, componentDefinition, compPtr))
                    {
                        break;
                    }
                }
            }

            Handle GetFirstActiveEntity() const;

            Handle GetNextActiveEntity(const Handle& handle) const;

            template <class ...TComponents>
            struct EntityComponentIter
            {
                EntityComponentIter(FixedArchetypeList* list, const Handle& handle);

                std::tuple<EntityId, TComponents...> operator*() const;

                EntityComponentIter& operator++();

                bool operator==(const EntityComponentIter& other) const;

                Handle Curr;
                FixedArchetypeList* List;
            };

            template <class ...TComponents>
            EntityComponentIter<TComponents...> begin()
            {
                return EntityComponentIter<TComponents...>(this, GetFirstActiveEntity());
            }

            template <class ...TComponents>
            EntityComponentIter<TComponents...> end()
            {
                return EntityComponentIter<TComponents...>(this, Handle());
            }

        private:

            // Gets the total offset to the entity at the given index in the raw Data buffer.
            uint32 GetOffsetToEntity(uint32 index) const;

            // Gets the total offset to the head of the components of an entity in the Data buffer.
            uint32 GetOffsetToEntityComponentHead(uint32 index) const;

            // Gets the total offset to the component of a given entity.
            uint32 GetOffsetToEntityComponent(uint32 index, const FName& componentId) const;

            // Gets a pointer to the Entity object at a given index.
            // Returns nullptr if there is no entity at that index.
            ArchetypeInstance* GetEntityPtr(uint32 index);

            // Gets a pointer to the Entity object at a given index.
            // Returns nullptr if there is no entity at that index.
            const ArchetypeInstance* GetEntityPtr(uint32 index) const;

            // Gets the address of the first component of an entity at a given index.
            void* GetEntityComponentHeadPtr(uint32 index);

            // Gets the address of the first component of an entity at a given index.
            const void* GetEntityComponentHeadPtr(uint32 index) const;

            // Gets the address of a component of an entity at a given index.
            // Returns nullptr if the entity or component is not valid.
            void* GetEntityComponentPtr(uint32 index, const FName& componentId);

            // Gets the address of a component of an entity at a given index.
            // Returns nullptr if the entity or component is not valid.
            const void* GetEntityComponentPtr(uint32 index, const FName& componentId) const;

            // Gets the address of a component of an entity at a given index.
            // Returns nullptr if the entity or component is not valid.
            template <class T>
            T* GetEntityComponentPtr(uint32 index)
            {
                void* dataPtr = GetEntityComponentPtr(index, T::StaticTypeName);
                return static_cast<T*>(dataPtr);
            }

            // Gets the address of a component of an entity at a given index.
            // Returns nullptr if the entity or component is not valid.
            template <class T>
            const T* GetEntityComponentPtr(uint32 index) const
            {
                const void* dataPtr = GetEntityComponentPtr(index, T::StaticTypeName);
                return static_cast<const T*>(dataPtr);
            }

            template <class T>
            T GetComponentRef(uint32 index)
            {
                auto ptr = GetEntityComponentPtr<typename std::remove_cv_t<typename std::remove_pointer_t<typename std::remove_reference_t<T>>>>(index);
                return ComponentAccessor<T>::GetComponentRef(ptr);
            }

            template <class T>
            T GetComponentRef(uint32 index) const
            {
                auto ptr = const_cast<typename std::remove_cv_t<typename std::remove_pointer_t<typename std::remove_reference_t<T>>>*>(GetEntityComponentPtr<typename std::remove_cv_t<typename std::remove_pointer_t<typename std::remove_reference_t<T>>>>(index));
                return ComponentAccessor<T>::GetComponentRef(ptr);
            }

            uint32 FindFreeSlot();

            uint32 Capacity = 0;
            uint32 Id = 0;
            ArchetypeDefinition Definition;
            uint32 NumInstances = 0;
            uint32 NumActiveInstances = 0;
            uint32 FreeSlotIndexHint = 0;
            TFixedStorage<uint8> Data;
        };

        template <class ...TComponents>
        struct EntityComponentSpan
        {
            static EntityComponentSpan FromList(FixedArchetypeList& list, uint32 startingIndex)
            {
                EntityComponentSpan span;
                span.RawData = static_cast<uint8*>(list.GetData());
                span.StartingIndex = startingIndex;
                span.InstanceCount = list.GetNumInstances();
                span.Step = list.GetEntityTotalSize();

                uint32 offsets[sizeof...(TComponents)] = { list.GetComponentLocalOffset(std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<TComponents>>>::StaticTypeName)... };
                memcpy(span.Offsets, offsets, sizeof...(TComponents) * sizeof(uint32));

                span.CheckRawData();

                return span;
            }

            uint32 GetStartIndex() const
            {
                CheckRawData();
                return StartingIndex;
            }

            uint32 GetInstanceCount() const
            {
                CheckRawData();
                return InstanceCount;
            }

            uint32 GetStep() const
            {
                CheckRawData();
                return Step;
            }

            uint32 GetGlobalIndex(uint32 localIndex) const
            {
                CheckRawData();
                return StartingIndex + localIndex;
            }

            std::tuple<EntityId, uint32, TComponents...> operator[](uint32 index) const
            {
                CheckRawData();
                uint32 entityOffset = index * Step;
                uint8* dataPtr = static_cast<uint8*>(RawData) + entityOffset;
                const ArchetypeInstance* instance = reinterpret_cast<const ArchetypeInstance*>(dataPtr);
                dataPtr += sizeof(ArchetypeInstance);
                return MakeTuple(instance->EntityId, index, dataPtr, std::make_index_sequence<sizeof...(TComponents)>{});
            }

            struct ConstIter
            {
                ConstIter(const EntityComponentSpan* span, uint32 index)
                    : Span(span)
                {
                    Span->CheckRawData();
                    Index = Span->FindNextActiveEntity(index);
                }

                std::tuple<EntityId, uint32, TComponents...> operator*() const
                {
                    Span->CheckRawData();
                    return Span->operator[](Index);
                }

                ConstIter& operator++()
                {
                    Span->CheckRawData();
                    Index = Span->FindNextActiveEntity(Index + 1);
                    return *this;
                }

                bool operator==(const ConstIter& other) const
                {
                    Span->CheckRawData();
                    return Span == other.Span && Index == other.Index;
                }

                uint32 Index;
                const EntityComponentSpan* Span;
            };

            void CheckRawData() const
            {
                uint64 d = reinterpret_cast<uint64>(RawData);
                if ((d & 0x000000FF00000000) == d)
                {
                    PHX_DEBUG_BREAK();
                }
            }

            ConstIter begin() const
            {
                CheckRawData();
                return ConstIter(this, 0);
            }

            ConstIter end() const
            {
                CheckRawData();
                return ConstIter(this, InstanceCount);
            }

        private:

            template <uint8 I, class T>
            T GetComponentRef(void* data) const
            {
                CheckRawData();
                auto ptr = reinterpret_cast<std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>*>(static_cast<uint8*>(data) + Offsets[I]);
                return ComponentAccessor<T>::GetComponentRef(ptr);
            }

            template <std::size_t ...Is>
            std::tuple<EntityId, uint32, TComponents...> MakeTuple(EntityId entityId, uint32 index, void* data, std::index_sequence<Is...>) const
            {
                CheckRawData();
                return { entityId, index, GetComponentRef<Is, TComponents>(data)... };
            }

            uint32 FindNextActiveEntity(uint32 index) const
            {
                CheckRawData();
                while (index < InstanceCount)
                {
                    uint32 entityOffset = index * Step;
                    const uint8* dataPtr = static_cast<uint8*>(RawData) + entityOffset;
                    const ArchetypeInstance* instance = reinterpret_cast<const ArchetypeInstance*>(dataPtr);
                    if (instance->EntityId != EntityId::Invalid)
                    {
                        break;
                    }
                    ++index;
                }
                return index;
            }

        public:

            uint32 StartingIndex = 0;
            uint32 InstanceCount = 0;
            uint32 Step = 0;
            uint32 Offsets[sizeof...(TComponents)] = {};
            void* RawData = nullptr;
        };
    }
}
