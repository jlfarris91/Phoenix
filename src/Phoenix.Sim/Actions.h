#pragma once

#include "Phoenix.Sim/Name.h"
#include "Phoenix.Sim/FixedPoint/FixedTypes.h"

namespace Phoenix
{
    enum class PHOENIX_SIM_API EDataType
    {
        UInt32,
        Int32,
        Name,
        Value,
        Distance,
        Degrees,
        Speed,
        Bool
    };

    union PHOENIX_SIM_API Data
    {
        uint32 AsUInt32;
        int32 AsInt32;
        FName AsName;
        Value AsValue;
        Distance AsDistance;
        Angle AsDegrees;
        Speed AsSpeed;
        bool AsBool;
    };

    bool operator==(const Data& a, const Data& b);

    struct PHOENIX_SIM_API TypedData
    {
        EDataType Type;
        Data Value;
    };

    struct PHOENIX_SIM_API Action
    {
        FName Verb = FName::None;
        FName Sender = FName::None;
        Data Args[6] = {};

        bool operator==(const Action& other) const;
        bool operator!=(const Action& other) const;
    };
}
