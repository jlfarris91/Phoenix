
#pragma once

#include "PhoenixSim/Containers/FixedSortedList.h"
#include "PhoenixSim/ECS/EntityId.h"

namespace Phoenix::ECS
{
    struct PHOENIX_SIM_API GroupEntity
    {
        GroupEntity() = default;
        GroupEntity(EntityId group, EntityId entity = EntityId::Invalid);

        bool operator==(const GroupEntity& other) const;

        bool IsValid() const;
        void Invalidate();

        EntityId Group;
        EntityId Entity;

        struct GetItemKey
        {
            EntityId operator()(const GroupEntity& item) const;
        };
    };

    class PHOENIX_SIM_API FixedGroupList
    {
    public:

        PHX_DECLARE_BLOCK_CONTAINER(FixedGroupList)
        {
            uint32 Capacity;
        };

        using TItem = GroupEntity;
        using TStorage = TFixedSortedList<GroupEntity, GroupEntity::GetItemKey>;

        uint32 GetCapacity() const;

        const GroupEntity* GetData() const;

        void Sort();

        uint32 GetNumValidPairs() const;

        bool ContainsEntity(EntityId group, EntityId entity) const;

        bool AddEntity(EntityId group, EntityId entity);

        bool RemoveEntity(EntityId group, EntityId entity);

        bool RemoveEntityFromAllGroups(EntityId entity);

        uint32 ClearGroup(EntityId group);

        uint32 GetNumEntities(EntityId group) const;

        EntityId GetFirstEntity(EntityId group, uint32& outIndex) const;

        EntityId GetNextEntity(EntityId group, uint32 currIndex, uint32& outIndex) const;

        template <class TCallback>
        void ForEach(const TCallback& callback) const
        {
            Storage.ForEachItem(callback);
        }

        template <class TCallback>
        void ForEachEntity(EntityId group, const TCallback& callback) const
        {
            Storage.ForEachItemProjected(
                group,
                [&](const GroupEntity& item) -> const EntityId& { return item.Entity; },
                callback);
        }

    private:

        struct RemoveEntityFromAllGroupsPred
        {
            EntityId Entity;
            EntityId operator()(const GroupEntity& item) const;
        };

        TStorage Storage;
    };
}
