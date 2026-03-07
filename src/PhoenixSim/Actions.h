#pragma once

#include "PhoenixSim/Name.h"
#include "PhoenixSim/FixedPoint/FixedTypes.h"

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
        uint32 UInt32;
        int32 Int32;
        FName Name;
        Value Value;
        Distance Distance;
        Angle Degrees;
        Speed Speed;
        bool Bool;
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
        Data Data[6] = {};

        bool operator==(const Action& other) const;
        bool operator!=(const Action& other) const;
    };
}
