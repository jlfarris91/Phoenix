
#pragma once

#include "PhoenixSim/ECS/EntityId.h"
#include "PhoenixSim/Reflection/GenericValue.h"

namespace Phoenix::RTS
{
    PHX_ECS_DECLARE_ENTITY_ID_SPEC(Unit);
}

// Teach the reflection system to treat UnitId as a plain uint32, matching EntityId's treatment.
namespace Phoenix
{
    template <>
    struct GenericValueTypeBuilder<RTS::UnitId>
    {
        static EGenericValueType GetPropertyValueType()
        {
            return EGenericValueType::UInt32;
        }
    };

    template <>
    struct GenericConverter<RTS::UnitId>
    {
        static GenericValue Borrow(RTS::UnitId v)
        {
            GenericValue gv;
            gv.Type.Primitive = EGenericValueType::UInt32;
            const uint32_t raw = static_cast<uint32_t>(v);
            std::memcpy(gv.Buffer, &raw, sizeof(raw));
            return gv;
        }

        static RTS::UnitId From(const GenericValue& gv)
        {
            uint32_t raw = 0;
            std::memcpy(&raw, gv.Buffer, sizeof(raw));
            return RTS::UnitId(raw);
        }
    };
}