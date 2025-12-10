#pragma once

#include "Name.h"
#include "PhoenixSim.h"
#include "FixedPoint/FixedTypes.h"

namespace Phoenix
{
    enum class EDataType
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

    union PHOENIXSIM_API Data
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

    struct PHOENIXSIM_API TypedData
    {
        EDataType Type;
        Data Value;
    };

    struct PHOENIXSIM_API Action
    {
        FName Verb = FName::None;
        FName Sender = FName::None;
        Data Data[6] = {};

        bool operator==(const Action& other) const;
        bool operator!=(const Action& other) const;
    };
}
