
#pragma once

#include "PhoenixSim/FixedPoint/FixedVector.h"

namespace Phoenix
{
    // Represents a ellipse in 2D space.
    struct PHOENIX_SIM_API Ellipse2
    {
        constexpr Ellipse2() = default;
        constexpr Ellipse2(const Vec2& origin, Distance radius) : Origin(origin), Radius(radius) {}
        constexpr Ellipse2(const Vec2& origin, const Vec2& radius) : Origin(origin), Radius(radius) {}

        Vec2 Origin;
        Vec2 Radius;
    };
}