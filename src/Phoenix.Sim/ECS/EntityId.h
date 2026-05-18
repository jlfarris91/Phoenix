
#pragma once

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Reflection/Registration.h"

namespace Phoenix::ECS
{
    typedef uint32 entityid_t;

    struct PHOENIX_SIM_API EntityId
    {
        static const EntityId Invalid;

        constexpr EntityId() = default;
        constexpr EntityId(entityid_t raw) : Id(raw) {}
        constexpr EntityId(const EntityId& other) = default;

        constexpr operator entityid_t() const { return Id; }
        constexpr EntityId& operator=(const entityid_t& id) { Id = id; return *this; }

        constexpr bool operator==(const EntityId& other) const { return Id == other.Id; }
        constexpr bool operator!=(const EntityId& other) const { return Id != other.Id; }

    private:
        entityid_t Id = -1;
    };
}

namespace std
{
    template <>
    struct hash<Phoenix::ECS::EntityId>
    {
        std::size_t operator()(const Phoenix::ECS::EntityId& entityId) const noexcept
        {
            return std::hash<Phoenix::ECS::entityid_t>{}(entityId);
        }
    };
}

#define PHX_ECS_DECLARE_ENTITY_ID_SPEC_WITH_BASE(name, base) \
    struct name##Id : base##Id \
    { \
        constexpr name##Id() = default; \
        constexpr explicit name##Id(ECS::entityid_t raw) : base##Id(raw) {} \
        constexpr explicit name##Id(const ECS::EntityId& id) : base##Id(id) {} \
    }

#define PHX_ECS_DECLARE_ENTITY_ID_SPEC(name) PHX_ECS_DECLARE_ENTITY_ID_SPEC_WITH_BASE(name, ECS::Entity)

