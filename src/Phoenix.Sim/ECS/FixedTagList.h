
#pragma once

#include "Phoenix.Sim/Name.h"
#include "Phoenix.Sim/Containers/FixedSortedList.h"
#include "Phoenix.Sim/ECS/EntityId.h"

namespace Phoenix::ECS
{
    struct PHOENIX_SIM_API EntityTag
    {
        EntityTag() = default;
        EntityTag(EntityId entity, FName tag = FName::None);

        bool operator==(const EntityTag& other) const;

        bool IsValid() const;
        void Invalidate();

        EntityId Entity;
        FName Tag;

        struct GetItemKey
        {
            EntityId operator()(const EntityTag& item) const;
        };
    };

    class PHOENIX_SIM_API FixedTagList
    {
    public:

        PHX_DECLARE_BLOCK_CONTAINER(FixedTagList)
        {
            uint32 Capacity;
        };

        using TItem = EntityTag;
        using TStorage = TFixedSortedList<EntityTag, EntityTag::GetItemKey>;

        uint32 GetCapacity() const;

        const EntityTag* GetData() const;

        void Sort();

        uint32 GetNumValidTags() const;

        bool HasTag(EntityId entity, const FName& tag) const;

        bool AddTag(EntityId entity, const FName& tag);

        bool RemoveTag(EntityId entity, const FName& tag);

        uint32 RemoveAllTags(EntityId entity);

        FName GetFirstTag(EntityId entity, uint32& outIndex) const;

        FName GetNextTag(EntityId entity, uint32 currIndex, uint32& outIndex) const;

        template <class TCallback>
        void ForEach(const TCallback& callback) const
        {
            Storage.ForEachItem(callback);
        }

        template <class TCallback>
        void ForEachTag(EntityId entity, const TCallback& callback) const
        {
            Storage.ForEachItemProjected(
                entity,
                [&](const EntityTag& item) -> const FName& { return item.Tag; },
                callback);
        }

    private:

        TStorage Storage;
    };
}
