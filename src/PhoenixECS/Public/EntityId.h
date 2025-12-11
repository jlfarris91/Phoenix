
#pragma once

#include "Platform.h"
#include "DLLExport.h"

namespace Phoenix
{
    namespace ECS
    {
        typedef uint32 entityid_t;
        
        struct PHOENIXECS_API EntityId
        {
            static const EntityId Invalid;

            constexpr EntityId() : Id(0) {}
            constexpr EntityId(entityid_t raw) : Id(raw) {}

            constexpr operator entityid_t() const { return Id; }
            constexpr EntityId& operator=(const entityid_t& id) { Id = id; return *this; }

            constexpr bool operator==(const EntityId& other) const { return Id == other.Id; }
            constexpr bool operator!=(const EntityId& other) const { return Id != other.Id; }

        private:
            entityid_t Id;
        };
    }
}
