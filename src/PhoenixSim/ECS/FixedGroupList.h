
#pragma once

#include "PhoenixSim/Containers/FixedSortedList.h"
#include "PhoenixSim/ECS/EntityId.h"

namespace Phoenix::ECS
{
    struct PHOENIX_SIM_API GroupEntity
    {
        GroupEntity() = default;
        GroupEntity(EntityId group, EntityId entity = EntityId::Invalid)
            : Group(group)
            , Entity(entity)
        {
        }

        bool operator==(const GroupEntity& other) const
        {
            return Group == other.Group && Entity == other.Entity;
        }

        bool IsValid() const { return Entity != EntityId::Invalid; }
        void Invalidate() { Entity = EntityId::Invalid; }

        EntityId Group;
        EntityId Entity;
    };

    template <size_t N>
    class FixedGroupList
    {
    public:

        static constexpr uint32 Capacity = N;

        uint32 GetSize() const
        {
            return Items.GetSize();
        }

        uint32 GetNumValidPairs() const
        {
            return Items.GetNumValidItems();
        }

        bool ContainsEntity(EntityId group, EntityId entity) const
        {
            return Items.Contains({ group, entity });
        }

        bool AddEntity(EntityId group, EntityId entity)
        {
            return Items.PushBackUnique({ group, entity });
        }

        bool RemoveEntity(EntityId group, EntityId entity)
        {
            return Items.Remove({ group, entity });
        }

        bool RemoveEntityFromAllGroups(EntityId entity)
        {
            return Items.RemoveAll(RemoveEntityFromAllGroupsPred { entity });
        }

        uint32 ClearGroup(EntityId group)
        {
            return Items.RemoveAll(group);
        }

        uint32 GetNumEntities(EntityId group) const
        {
            return Items.GetNumSubItems(group);
        }

        EntityId GetFirstEntity(EntityId group, uint32& outIndex) const
        {
            const GroupEntity* item = Items.GetFirstSubItem(group, outIndex);
            return item ? item->Entity : EntityId::Invalid;
        }

        EntityId GetNextEntity(EntityId group, uint32 currIndex, uint32& outIndex) const
        {
            const GroupEntity* item = Items.GetNextSubItem(group, currIndex, outIndex);
            return item ? item->Entity : EntityId::Invalid;
        }

        template <class TCallback>
        void ForEach(const TCallback& callback) const
        {
            Items.ForEachItem(callback);
        }

        template <class TCallback>
        void ForEachEntity(EntityId group, const TCallback& callback) const
        {
            Items.ForEachSubItem(group, [&](const GroupEntity& item) -> const EntityId&
            {
                return item.Entity;
            }, callback);
        }

        void Sort()
        {
            Items.Sort();
        }

    private:

        struct GetItemKey
        {
            EntityId operator()(const GroupEntity& item) const
            {
                return item.Group;
            }
        };

        struct RemoveEntityFromAllGroupsPred
        {
            EntityId Entity;
            EntityId operator()(const GroupEntity& item) const
            {
                return item.Entity == Entity;
            }
        };

        TFixedSortedList<GroupEntity, Capacity, GetItemKey> Items;
    };
}
