
#pragma once

#include "PhoenixSim/Utils.h"
#include "PhoenixSim/Containers/FixedArray.h"
#include "PhoenixSim/ECS/Entity.h"

namespace Phoenix::ECS
{
    class PHOENIX_SIM_API FixedEntityList
    {
    public:

        using TItem = Entity;
        using TStorage = TFixedArray<Entity>;

        FixedEntityList() = default;

        template <class TAllocator>
        FixedEntityList(TAllocator& allocator, uint32 capacity)
            : Storage(allocator, capacity)
        {
        }

        template <class TAllocator>
        FixedEntityList(TAllocator& allocator, uint32 capacity, const FixedEntityList& other)
            : Storage(allocator, capacity, other.Storage)
        {
        }

        uint32 GetCapacity() const;

        static uint32 GetAllocSizeBytes(uint32 capacity);

        uint32 GetAllocSizeBytes() const;

        const Entity* GetData() const;

        uint32 GetNumActive() const;

        uint32 GetNumHighWaterMark() const;

        bool IsValid(EntityId entityId) const;

        uint32 GetEntityIndex(EntityId entityId) const;

        Entity* GetEntityPtr(EntityId entityId);

        const Entity* GetEntityPtr(EntityId entityId) const;

        Entity& GetEntityRef(EntityId entityId);

        const Entity& GetEntityRef(EntityId entityId) const;

        EntityId Acquire(const FName& kind);

        bool Release(EntityId entityId);

        using FOnEntityReclaimed = TFunction<void(const EntityId& entityId)>;
        uint32 ReclaimEntities(const FOnEntityReclaimed& callback);

        template <class TCallback>
        void ForEach(const TCallback& callback) const
        {
            for (uint32 i = 0; i < Storage.GetNum(); ++i)
            {
                const Entity& entity = Storage[i];
                if (IsValid(entity.Id))
                {
                    InvokeForEachCallbackWithIndex(callback, i, entity);
                }
            }
        }

    private:

        bool IsMarkedForDeath(EntityId entityId, uint32 index) const;
        void ReclaimEntity(Entity& entity);

        TStorage Storage;
        uint32 NumActiveEntities = 0;
        uint32 NumHighWaterMark = 0;
    };
}
