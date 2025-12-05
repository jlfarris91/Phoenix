
#pragma once

#include "DLLExport.h"
#include "Name.h"
#include "Platform.h"
#include "FixedPoint/FixedTypes.h"

namespace Phoenix::LDS
{
    enum class ELDSValueType : uint8
    {
        Unknown,

        // Pod types
        Bool,
        Int32,
        UInt32,
        Name,
        Value,
        Distance,
        Degrees,
        Speed,
        Time,

        // Localized string
        Text,

        // Asset reference
        Asset,

        // Special types
        Enum,
        EnumFlags,
        Array,
        Object,     // A full object definition.
        ObjectRef,  // A reference to another object in the catalog.
        Expression  // A reference to code.
    };

    PHOENIX_LDS_API bool TryParse(const PHXString& string, ELDSValueType& outEnum);
    PHOENIX_LDS_API PHXString ToString(ELDSValueType valueType);

    union PHOENIX_LDS_API LDSValue
    {
        int32 Int32;
        uint32 UInt32;
        FName Name;
        Value Value;
        Distance Distance;
        Angle Degrees;
        Speed Speed;
        Time Time;
        bool Bool;
    };

    struct PHOENIX_LDS_API LDSTypedValue
    {
        constexpr LDSTypedValue() = default;

        constexpr LDSTypedValue(ELDSValueType type)
            : Value({ .UInt32 = (uint32)type })
            , Type(type)
        {
        }

        constexpr LDSTypedValue(const LDSValue& value, ELDSValueType type)
            : Value(value)
            , Type(type)
        {
        }

#define DEFINE_LDSTypedValue_CTOR(type, enumType) \
        constexpr LDSTypedValue(type value) \
            : Value{ .enumType = value } \
            , Type(ELDSValueType::enumType) {}

        DEFINE_LDSTypedValue_CTOR(int32, Int32)
        DEFINE_LDSTypedValue_CTOR(uint32, UInt32)
        DEFINE_LDSTypedValue_CTOR(FName, Name)
        DEFINE_LDSTypedValue_CTOR(Value, Value)
        // DEFINE_LDSTypedValue_CTOR(Distance, Distance);
        DEFINE_LDSTypedValue_CTOR(Angle, Degrees)
        DEFINE_LDSTypedValue_CTOR(Speed, Speed)

#undef DEFINE_LDSTypedValue_CTOR

        LDSValue Value = {};
        ELDSValueType Type = ELDSValueType::Unknown;
    };
}
