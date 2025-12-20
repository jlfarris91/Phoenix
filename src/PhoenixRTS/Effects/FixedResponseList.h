#pragma once

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Name.h"
#include "PhoenixSim/Containers/FixedSortedList.h"
#include "PhoenixSim/ECS/EntityId.h"

namespace Phoenix::RTS
{
    struct PHOENIX_RTS_API EntityResponse
    {
        EntityResponse() = default;
        EntityResponse(ECS::EntityId entity, const FName& response = FName::None)
            : Entity(entity)
            , Response(response)
        {
        }

        bool operator==(const EntityResponse& other) const
        {
            return Entity == other.Entity && Response == other.Response;
        }

        bool IsValid() const { return Response != FName::None; }
        void Invalidate() { Response = FName::None; }

        ECS::EntityId Entity;
        FName Response;
    };

    template <uint32 N>
    struct FixedResponseList
    {
        static constexpr uint32 Capacity = N;

        uint32 GetSize() const
        {
            return Items.GetSize();
        }

        uint32 GetNumValidResponses() const
        {
            return Items.GetNumValidItems();
        }

        bool ContainsResponse(ECS::EntityId entity, const FName& response) const
        {
            return Items.Contains({ entity, response });
        }

        bool AddResponse(ECS::EntityId entity, const FName& response)
        {
            return Items.PushBackUnique({ entity, response });
        }

        bool RemoveResponse(ECS::EntityId entity, const FName& response)
        {
            return Items.Remove({ entity, response });
        }

        uint32 ClearResponses(ECS::EntityId entity)
        {
            return Items.RemoveAll(entity);
        }

        FName GetFirstResponse(ECS::EntityId entity, uint32& outIndex) const
        {
            EntityResponse* item = Items.GetFirstSubItem(entity, outIndex);
            return item ? item->Response : FName::None;
        }

        FName GetNextResponse(ECS::EntityId entity, uint32 currIndex, uint32& outIndex) const
        {
            EntityResponse* item = Items.GetNextSubItem(entity, currIndex, outIndex);
            return item ? item->Response : FName::None;
        }

        template <class TCallback>
        void ForEach(const TCallback& callback) const
        {
            Items.ForEachItem([&](const EntityResponse& item)
            {
                callback(item);
            });
        }

        template <class TCallback>
        void ForEachResponse(ECS::EntityId entity, const TCallback& callback) const
        {
            Items.ForEachSubItem(entity, [&](const EntityResponse& item)
            {
                callback(item.Response);
            });
        }

        void Sort()
        {
            Items.Sort();
        }

    private:

        struct GetItemKey
        {
            ECS::EntityId operator()(const EntityResponse& item) const
            {
                return item.Entity;
            }
        };

        TFixedSortedList<EntityResponse, Capacity, GetItemKey> Items;
    };
    
}
