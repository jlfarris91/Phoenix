#pragma once

#include "PhoenixSim/Color.h"
#include "PhoenixSim/FixedPoint/FixedVector.h"
#include "PhoenixSim/Name.h"

namespace Phoenix::Debug::Commands
{
    struct DrawRay
    {
        static constexpr FName StaticId = "Debug_DrawRay"_n;
        Vec2 Start;
        Vec2 Dir;
        Color Color;
    };

    struct DrawCircle
    {
        static constexpr FName StaticId = "Debug_DrawCircle"_n;
        Vec2 Center;
        Distance Radius;
        Color Color;
    };
}
