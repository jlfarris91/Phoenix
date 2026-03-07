
#pragma once

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/ECS/EntityId.h"

namespace Phoenix
{
    namespace ECS
    {
        struct PHOENIX_SIM_API ArchetypeHandle
        {
            friend class FixedArchetypeList;

            ArchetypeHandle() = default;

            ArchetypeHandle(EntityId entityId) : EntityId(entityId) {}

            uint32 GetOwnerId() const
            {
                return OwnerId;
            }

            constexpr EntityId GetEntityId() const
            {
                return EntityId;
            }

            bool operator==(const ArchetypeHandle& other) const = default;

        private:

            ArchetypeHandle(uint32 ownerId, uint32 id, EntityId entityId)
                : OwnerId(ownerId)
                , Id(id)
                , EntityId(entityId)
            {
            }

            uint32 OwnerId = Index<uint32>::None;
            uint32 Id = Index<uint32>::None;
            EntityId EntityId = EntityId::Invalid;
        };
    }
}