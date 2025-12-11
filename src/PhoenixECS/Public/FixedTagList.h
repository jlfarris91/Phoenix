
#pragma once

#include "EntityId.h"
#include "Containers/FixedArray.h"
#include "Containers/FixedSortedList.h"

namespace Phoenix::ECS
{
    template <size_t N>
    class FixedTagList
    {
    public:

        static constexpr uint32 Capacity = N;

        uint32 GetSize() const
        {
            return Items.GetSize();
        }

        uint32 GetNumValidTags() const
        {
            return Items.GetNumValidItems();
        }

        bool HasTag(EntityId entity, const FName& tag) const
        {
            return Items.Contains({ entity, tag });
        }

        bool AddTag(EntityId entity, const FName& tag)
        {
            return Items.PushBackUnique({ entity, tag });
        }

        bool RemoveTag(EntityId entity, const FName& tag)
        {
            return Items.Remove({ entity, tag });
        }

        uint32 RemoveAllTags(EntityId entity)
        {
            return Items.RemoveAll(entity);
        }

        FName GetFirstTag(EntityId entity, uint32& outIndex) const
        {
            EntityTag* item = Items.GetFirstSubItem(entity, outIndex);
            return item ? item->Tag : FName::None;
        }

        FName GetNextTag(EntityId entity, uint32 currIndex, uint32& outIndex) const
        {
            EntityTag* item = Items.GetNextSubItem(entity, currIndex, outIndex);
            return item ? item->Tag : FName::None;
        }

        template <class TCallback>
        void ForEachTag(EntityId entity, const TCallback& callback) const
        {
            Items.ForEachSubItem(entity, [&](const EntityTag& item)
            {
                callback(item.Tag);
            });
        }

        void Sort()
        {
            Items.Sort();
        }

    private:

        struct EntityTag
        {
            EntityTag() = default;
            EntityTag(EntityId entity, FName tag = FName::None)
                : Entity(entity)
                , Tag(tag)
            {
            }

            bool operator==(const EntityTag& other) const
            {
                return Entity == other.Entity && Tag == other.Tag;
            }

            bool IsValid() const { return Tag != FName::None; }
            void Invalidate() { Tag = FName::None; }

            EntityId Entity;
            FName Tag;
        };

        struct GetItemKey
        {
            EntityId operator()(const EntityTag& item) const
            {
                return item.Entity;
            }
        };

        TFixedSortedList<EntityTag, GetItemKey, TFixedArray<EntityTag, Capacity>> Items;
    };
}
