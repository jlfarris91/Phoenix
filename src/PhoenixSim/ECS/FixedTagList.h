
#pragma once

#include "PhoenixSim/Name.h"
#include "PhoenixSim/Containers/FixedSortedList.h"
#include "PhoenixSim/ECS/EntityId.h"

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

        using TItem = EntityTag;
        using TStorage = TFixedSortedList<EntityTag, EntityTag::GetItemKey>;

        FixedTagList() = default;

        template <class TAllocator>
        FixedTagList(TAllocator& allocator, uint32 capacity)
            : Storage(allocator, capacity)
        {
        }

        template <class TAllocator>
        FixedTagList(TAllocator& allocator, uint32 capacity, const FixedTagList& other)
            : Storage(allocator, capacity, other.Storage)
        {
        }

        uint32 GetCapacity() const;

        static uint32 GetAllocSizeBytes(uint32 capacity);

        uint32 GetAllocSizeBytes() const;

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
