
#pragma once

#include "EntityId.h"
#include "Containers/FixedArray.h"
#include "Containers/FixedSortedList.h"

namespace Phoenix::ECS
{
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

        uint32 RemoveAllEntities(EntityId group)
        {
            return Items.RemoveAll(group);
        }

        EntityId GetFirstEntity(EntityId group, uint32& outIndex) const
        {
            GroupEntity* item = Items.GetFirstItem(group, outIndex);
            return item ? item->Entity : EntityId::Invalid;
        }

        EntityId GetNextEntity(EntityId group, uint32 currIndex, uint32& outIndex) const
        {
            GroupEntity* item = Items.GetNextItem(group, currIndex, outIndex);
            return item ? item->Entity : EntityId::Invalid;
        }

        template <class TCallback>
        void ForEachEntity(EntityId group, const TCallback& callback) const
        {
            Items.ForEachItem(group, [&](const GroupEntity& item)
            {
                callback(item.Entity);
            });
        }

        void Sort()
        {
            Items.Sort();
        }

    private:

        struct GroupEntity
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

        struct GetItemKey
        {
            EntityId operator()(const GroupEntity& item) const
            {
                return item.Group;
            }
        };

        TFixedSortedList<GroupEntity, GetItemKey, TFixedArray<GroupEntity, Capacity>> Items;
    };
}
