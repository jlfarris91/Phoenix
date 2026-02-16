
#pragma once

#include "PhoenixSim/FixedPoint/FixedVector.h"

namespace Phoenix
{
    // Represents a circle in 2D space.
    struct PHOENIX_SIM_API Circle2
    {
        constexpr Circle2() = default;
        constexpr Circle2(const Vec2& origin, Distance radius) : Origin(origin), Radius(radius) {}

        Vec2 Origin;
        Distance Radius;
    };
}