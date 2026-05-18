
#pragma once

#include "PhoenixSim/FixedPoint/FixedVector.h"
#include "PhoenixSim/Reflection/Registration.h"

namespace Phoenix
{
    // Represents a circle in 2D space.
    template <class TVec>
    struct PHOENIX_SIM_API TCircle
    {
        PHX_DECLARE_TYPE(TCircle)

        constexpr TCircle() = default;
        constexpr TCircle(const TVec& origin, TVec::ComponentT radius) : Origin(origin), Radius(radius) {}

        TVec Origin;
        TVec::ComponentT Radius;
    };

    typedef TCircle<Vec2> Circle2;
}

PHX_DEFINE_TYPE(Phoenix::Circle2)
{
    registration
        .Alias("Circle2")
        .Namespace("Phoenix.Circle2")
        .Constructor<const Vec2&, Vec2::ComponentT>()
        .Field("Origin", &Circle2::Origin)
        .Field("Radius", &Circle2::Radius);
}