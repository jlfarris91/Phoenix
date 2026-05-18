#pragma once

#include "Phoenix.Sim.RTS/DLLExport.h"
#include "Phoenix.Sim/Platform.h"
#include "Phoenix.Sim/Name.h"
#include "Phoenix.Sim/Containers/FixedSortedList.h"
#include "Phoenix.Sim/ECS/EntityId.h"

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

        struct GetItemKey
        {
            ECS::EntityId operator()(const EntityResponse& item) const
            {
                return item.Entity;
            }
        };
    };

    class PHOENIX_RTS_API FixedResponseList
    {
        using TStorage = TFixedSortedList<EntityResponse, EntityResponse::GetItemKey>;

    public:
        
        PHX_DECLARE_BLOCK_CONTAINER(FixedResponseList)
        {
            uint32 Capacity;
        };

        uint32 GetCapacity() const;

        uint32 GetNum() const;

        uint32 GetNumValidResponses() const;

        bool ContainsResponse(ECS::EntityId entity, const FName& response) const;

        bool AddResponse(ECS::EntityId entity, const FName& response);

        bool RemoveResponse(ECS::EntityId entity, const FName& response);

        uint32 ClearResponses(ECS::EntityId entity);

        FName GetFirstResponse(ECS::EntityId entity, uint32& outIndex) const;

        FName GetNextResponse(ECS::EntityId entity, uint32 currIndex, uint32& outIndex) const;

        template <class TCallback>
        void ForEach(const TCallback& callback) const
        {
            Storage.ForEachItem(callback);
        }

        template <class TCallback>
        void ForEachResponse(ECS::EntityId entity, const TCallback& callback) const
        {
            Storage.ForEachItemProjected(entity, [&](const EntityResponse& item) -> const FName&
            {
                return item.Response;
            }, callback);
        }

        void Sort();

    private:

        TStorage Storage;
    };
}
