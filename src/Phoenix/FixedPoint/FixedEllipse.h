
#pragma once

#include "Phoenix/FixedPoint/FixedVector.h"
#include "Phoenix/Reflection/Registration.h"

namespace Phoenix
{
    // Represents a ellipse in 2D space.
    template <class TVec>
    struct PHOENIX_SIM_API TEllipse
    {
        PHX_DECLARE_TYPE(TEllipse)

        constexpr TEllipse() = default;
        constexpr TEllipse(const TVec& origin, TVec::ComponentT radius) : Origin(origin), Radius(radius) {}
        constexpr TEllipse(const TVec& origin, const TVec& radius) : Origin(origin), Radius(radius) {}

        TVec Origin;
        TVec Radius;
    };

    typedef TEllipse<Vec2> Ellipse2;
}

PHX_DEFINE_TYPE(Phoenix::Ellipse2)
{
    registration
        .Alias("Ellipse2")
        .Namespace("Phoenix.Ellipse2")
        .Constructor<const Vec2&, Vec2::ComponentT>()
        .Constructor<const Vec2&, const Vec2&>()
        .Field("Origin", &Ellipse2::Origin)
        .Field("Radius", &Ellipse2::Radius);
}